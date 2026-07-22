#pragma once

#include <Arduino.h>
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "ButtonManager.h"
#include "RFIDManager.h"
#include "blynk_manager.h"

// Wires SensorManager + ActuatorManager + ButtonManager + RFIDManager + BlynkManager
// together into the 6 home-automation rules. Owns no hardware pins itself - only orchestration state.
//
// Design principle used for every "auto vs manual override" rule (roof, fan):
// automation only acts on the EDGE of a condition becoming true/false, not every loop.
// This means a manual button press (physical OR from the Blynk app) in between two edges
// is never immediately overwritten by the automatic rule - it persists until the next
// real-world condition change.
class HomeController
{
public:
    void begin();

    // Call every loop(). Reads current sensor/button/RFID/Blynk state and drives the actuators.
    void update(SensorManager &sensors, ActuatorManager &actuators,
                ButtonManager &buttons, RFIDManager &rfid, BlynkManager &blynk);

private:
    // Logic 2: rain -> roof (edge-triggered automation, button/app always free to override)
    void handleRoof(SensorManager &sensors, ActuatorManager &actuators,
                    ButtonManager &buttons, BlynkManager &blynk);

    // Logic 3: RFID -> door open, wrong card -> buzzer beep. Logic 5 also opens the door (gas).
    void handleDoorAndRFID(SensorManager &sensors, ActuatorManager &actuators,
                           ButtonManager &buttons, RFIDManager &rfid, BlynkManager &blynk);

    // Logic 4: temperature -> fan (edge-triggered automation, button/app always free to override)
    void handleFan(SensorManager &sensors, ActuatorManager &actuators,
                   ButtonManager &buttons, BlynkManager &blynk);

    // Logic 5: gas -> buzzer + auto-open door (sets flags consumed by handleDoorAndRFID/buzzer resolver)
    void handleGasAlarm(SensorManager &sensors, ActuatorManager &actuators);

    // Closes the door GAS_DOOR_AUTO_CLOSE_DELAY_MS after gas clears, unless cancelled
    // by a manual door action (button/app/RFID) or a new gas alarm in the meantime.
    void handleGasDoorAutoClose(ActuatorManager &actuators);

    // Logic 6: darkness + motion -> light, with a hold timer so it doesn't flicker off
    // the instant the person stops moving. No manual override (per project decision).
    void handleLight(SensorManager &sensors, ActuatorManager &actuators);

    // Buzzer AND alert LED share the same two alarm triggers (gas alarm + wrong-card 3-strike) -
    // resolved once per loop. The alert LED only reflects real alarms; routine beep feedback
    // (success/error chirps) only drives the buzzer, not the LED.
    void resolveAlarmOutputs(ActuatorManager &actuators);

    // Non-blocking beep pattern player: N short beeps of onMs/offMs, used for RFID
    // success/error feedback. Call startBeep() once, isBeepCurrentlyOn() every loop.
    void startBeep(int count, unsigned long onMs, unsigned long offMs);
    bool isBeepCurrentlyOn();

    // Pushes Temp/Gas values + the 2 Blynk events every loop (each throttles/edge-detects itself).
    void updateBlynk(SensorManager &sensors, BlynkManager &blynk);

    // ---- Roof (Logic 2) ----
    bool _wasRaining = false;

    // ---- Fan (Logic 4) ----
    // Tracks the *temperature condition* only, never touched by button presses -
    // this is what makes edge-detection immune to manual overrides changing the actual fan state.
    bool _tempAboveThreshold = false;

    // ---- Gas alarm (Logic 5) ----
    bool _gasAlarmActive = false;
    bool _gasDoorCloseScheduled = false;
    unsigned long _gasDoorCloseAt = 0;

    // ---- Wrong-card buzzer pulse (Logic 3) ----
    bool _wrongCardBeepActive = false;
    unsigned long _wrongCardBeepStarted = 0;
    int _wrongCardCount = 0;

    // ---- Light (Logic 6) ----
    unsigned long _lastMotionWhileDark = 0;

    // ---- Transient beep pattern (RFID success/error feedback) ----
    bool _beepActive = false;
    unsigned long _beepStartedAt = 0;
    int _beepCount = 0;
    unsigned long _beepOnMs = 0;
    unsigned long _beepOffMs = 0;
};