#pragma once

#include <Arduino.h>
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "ButtonManager.h"
#include "RFIDManager.h"

// Wires SensorManager + ActuatorManager + ButtonManager + RFIDManager together
// into the 6 home-automation rules. Owns no hardware pins itself - only orchestration state.
//
// Design principle used for every "auto vs manual override" rule (roof, fan):
// automation only acts on the EDGE of a condition becoming true/false, not every loop.
// This means a manual button press in between two edges is never immediately overwritten
// by the automatic rule - it persists until the next real-world condition change.
class HomeController
{
public:
    void begin();

    // Call every loop(). Reads current sensor/button/RFID state and drives the actuators.
    void update(SensorManager &sensors, ActuatorManager &actuators,
                ButtonManager &buttons, RFIDManager &rfid);

private:
    // Logic 2: rain -> roof (edge-triggered automation, button always free to override)
    void handleRoof(SensorManager &sensors, ActuatorManager &actuators, ButtonManager &buttons);

    // Logic 3: RFID -> door open, wrong card -> buzzer beep. Logic 5 also opens the door (gas).
    void handleDoorAndRFID(SensorManager &sensors, ActuatorManager &actuators,
                           ButtonManager &buttons, RFIDManager &rfid);

    // Logic 4: temperature -> fan (edge-triggered automation, button always free to override)
    void handleFan(SensorManager &sensors, ActuatorManager &actuators, ButtonManager &buttons);

    // Logic 5: gas -> buzzer + auto-open door (sets flags consumed by handleDoorAndRFID/buzzer resolver)
    void handleGasAlarm(SensorManager &sensors, ActuatorManager &actuators);

    // Closes the door GAS_DOOR_AUTO_CLOSE_DELAY_MS after gas clears, unless cancelled
    // by a manual door action (button/RFID) or a new gas alarm in the meantime.
    void handleGasDoorAutoClose(ActuatorManager &actuators);

    // Logic 6: darkness + motion -> light, with a hold timer so it doesn't flicker off
    // the instant the person stops moving. No manual override (per project decision).
    void handleLight(SensorManager &sensors, ActuatorManager &actuators);

    // Buzzer AND alert LED share the same two triggers (gas alarm + wrong RFID card) -
    // resolved once per loop as a simple OR of both sources, so they never fight each other.
    void resolveAlarmOutputs(ActuatorManager &actuators);

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
};