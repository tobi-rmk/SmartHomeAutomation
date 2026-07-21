#pragma once

// Template info from your Blynk.Console project. This block (together with
// BLYNK_AUTH_TOKEN from secrets.h) MUST be defined before <BlynkSimpleEsp32.h> is
// included anywhere - blynk_manager.cpp is the only file that includes that header.
#define BLYNK_TEMPLATE_ID "TMPL6_Jd8ONNt"
#define BLYNK_TEMPLATE_NAME "FinalSmartHome"

#include <Arduino.h>

// Virtual pin map - must match the datastreams configured in your Blynk template:
//   V0 Temp  (value, read-only)      V1 Gas   (value, read-only)
//   V2 Fan   (switch, two-way sync)  V3 Door  (switch, two-way sync)  V4 Roof (switch, two-way sync)
#define VPIN_TEMP V0
#define VPIN_GAS V1
#define VPIN_FAN V2
#define VPIN_DOOR V3
#define VPIN_ROOF V4

// Owns the Blynk connection.
//  - Pushes Temp/Gas values to the dashboard (throttled).
//  - Keeps the 3 switch widgets in sync with the real actuator state, from ANY source.
//  - Fires the 2 alert events, edge-triggered (once per rising edge, not spammed every loop).
//  - Exposes "command from app" as one-shot events, consumed the same way HomeController
//    consumes ButtonManager presses - so an app toggle behaves exactly like a physical button.
class BlynkManager
{
public:
    void begin();
    void run(); // call every loop()

    void updateSensors(float temperature, int gasValue);

    void syncFan(bool on);
    void syncDoor(bool on);
    void syncRoof(bool on);

    // Edge-triggered - call every loop with the current alarm state.
    void notifyGasAlert(bool active);
    void notifyDoorAlert(bool active);

    // Returns true at most once per app action, fills outValue with the requested state.
    bool consumeFanCommand(bool &outValue);
    bool consumeDoorCommand(bool &outValue);
    bool consumeRoofCommand(bool &outValue);

    // Called internally by the global BLYNK_WRITE callbacks - not meant to be called directly.
    void _setFanCommand(bool value);
    void _setDoorCommand(bool value);
    void _setRoofCommand(bool value);

private:
    unsigned long _lastSensorPush = 0;
    bool _gasAlertActive = false;
    bool _doorAlertActive = false;

    bool _fanCommandPending = false;
    bool _fanCommandValue = false;
    bool _doorCommandPending = false;
    bool _doorCommandValue = false;
    bool _roofCommandPending = false;
    bool _roofCommandValue = false;
};

// Single global instance - required because Blynk's BLYNK_WRITE macros expand into
// free functions at global scope, which need a way to reach this class.
extern BlynkManager blynkManager;