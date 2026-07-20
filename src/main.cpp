#include <Arduino.h>
#include "config.h"
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "ButtonManager.h"
#include "DisplayManager.h"
#include "RFIDManager.h"
#include "HomeController.h"

SensorManager sensors;
ActuatorManager actuators;
ButtonManager buttons;
DisplayManager display;
RFIDManager rfid;
HomeController home;

unsigned long lastSensorRead = 0;

void setup()
{
    Serial.begin(115200);

    display.begin();
    sensors.begin();
    actuators.begin();
    buttons.begin();
    rfid.begin();
    home.begin();

    // TODO: replace with your own card's real UID (see test/main_stageC.backup.cpp to scan it)
    byte authorizedUID[] = {0x10, 0xCC, 0xD6, 0x0B};
    rfid.setAuthorizedUID(authorizedUID, sizeof(authorizedUID));

    display.runWarmup();
}

void loop()
{
    unsigned long now = millis();

    buttons.update();

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorRead = now;
        sensors.update();
    }

    // HomeController reads current sensor/button/RFID state every loop and drives
    // all 6 automation rules (roof, door/RFID, fan, gas alarm, light) + resolves the buzzer.
    home.update(sensors, actuators, buttons, rfid);

    // DisplayManager throttles itself internally (DISPLAY_UPDATE_MS) - safe to call every loop
    display.update(sensors.getTemperature(), sensors.isRaining(), sensors.getGasValue(), sensors.isDark());
}