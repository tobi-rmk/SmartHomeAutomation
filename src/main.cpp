#include <Arduino.h>
#include "config.h"
#include "secrets.h"
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "ButtonManager.h"
#include "DisplayManager.h"
#include "RFIDManager.h"
#include "blynk_manager.h"
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

    // Blynk connects AFTER the warmup so the LCD countdown isn't delayed by a slow WiFi handshake
    blynkManager.begin();
}

void loop()
{
    unsigned long now = millis();

    buttons.update();
    blynkManager.run();

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorRead = now;
        sensors.update();
    }

    // HomeController reads current sensor/button/RFID/Blynk state every loop and drives
    // all 6 automation rules + syncs the dashboard + fires the 2 alert events.
    home.update(sensors, actuators, buttons, rfid, blynkManager);

    // DisplayManager throttles itself internally (DISPLAY_UPDATE_MS) - safe to call every loop
    display.update(sensors.getTemperature(), sensors.isRaining(), sensors.getGasValue(), sensors.isDark());
}