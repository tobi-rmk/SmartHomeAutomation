#include "blynk_manager.h"
#include "secrets.h"
#include <BlynkSimpleEsp32.h>
#include "config.h"

BlynkManager blynkManager;

void BlynkManager::begin()
{
    // Blocking on first connect - acceptable at startup, right after the LCD warmup.
    Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
}

void BlynkManager::run()
{
    Blynk.run();
}

void BlynkManager::updateSensors(float temperature, int gasValue, bool motionDetected, bool isRaining)
{
    unsigned long now = millis();
    if (now - _lastSensorPush < BLYNK_SENSOR_PUSH_MS)
        return;
    _lastSensorPush = now;

    Blynk.virtualWrite(VPIN_TEMP, temperature);
    Blynk.virtualWrite(VPIN_GAS, gasValue);
    Blynk.virtualWrite(VPIN_MOTION, motionDetected ? 1 : 0); // LED widget: 1 = on, 0 = off
    Blynk.virtualWrite(VPIN_RAIN, isRaining ? 1 : 0);        // LED widget: 1 = on, 0 = off
}

void BlynkManager::syncFan(bool on) { Blynk.virtualWrite(VPIN_FAN, on ? 1 : 0); }
void BlynkManager::syncDoor(bool on) { Blynk.virtualWrite(VPIN_DOOR, on ? 1 : 0); }
void BlynkManager::syncRoof(bool on) { Blynk.virtualWrite(VPIN_ROOF, on ? 1 : 0); }

void BlynkManager::notifyGasAlert(bool active)
{
    if (active && !_gasAlertActive)
    {
        Blynk.logEvent("gas_alert", "Gas level exceeded the safe threshold!");
    }
    _gasAlertActive = active;
}

void BlynkManager::notifyDoorAlert(bool active)
{
    if (active && !_doorAlertActive)
    {
        Blynk.logEvent("door_alert", "3 incorrect RFID attempts detected at the door!");
    }
    _doorAlertActive = active;
}

bool BlynkManager::consumeFanCommand(bool &outValue)
{
    if (!_fanCommandPending)
        return false;
    outValue = _fanCommandValue;
    _fanCommandPending = false;
    return true;
}
bool BlynkManager::consumeDoorCommand(bool &outValue)
{
    if (!_doorCommandPending)
        return false;
    outValue = _doorCommandValue;
    _doorCommandPending = false;
    return true;
}
bool BlynkManager::consumeRoofCommand(bool &outValue)
{
    if (!_roofCommandPending)
        return false;
    outValue = _roofCommandValue;
    _roofCommandPending = false;
    return true;
}

void BlynkManager::_setFanCommand(bool value)
{
    _fanCommandPending = true;
    _fanCommandValue = value;
}
void BlynkManager::_setDoorCommand(bool value)
{
    _doorCommandPending = true;
    _doorCommandValue = value;
}
void BlynkManager::_setRoofCommand(bool value)
{
    _roofCommandPending = true;
    _roofCommandValue = value;
}

// ==================== Blynk callbacks - must be free functions at global scope ====================
BLYNK_WRITE(VPIN_FAN) { blynkManager._setFanCommand(param.asInt() == 1); }
BLYNK_WRITE(VPIN_DOOR) { blynkManager._setDoorCommand(param.asInt() == 1); }
BLYNK_WRITE(VPIN_ROOF) { blynkManager._setRoofCommand(param.asInt() == 1); }

// Ask the server to re-push the 3 switch states to the app whenever the device (re)connects,
// so the dashboard doesn't show a stale ON/OFF after a reboot or WiFi drop.
BLYNK_CONNECTED()
{
    Blynk.syncVirtual(VPIN_FAN, VPIN_DOOR, VPIN_ROOF);
}