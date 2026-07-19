#include <Arduino.h>
#include "config.h"
#include "SensorManager.h"
#include "DisplayManager.h"
// #include "ButtonManager.h"

SensorManager sensors;
DisplayManager display;
// ButtonManager buttons;

unsigned long lastSensorRead = 0;

void setup()
{
    Serial.begin(115200);

    display.begin();
    sensors.begin();
    // buttons.begin();

    display.runWarmup();
}

void loop()
{
    unsigned long now = millis();

    // buttons.update();

    // // TODO: once ActuatorManager has been tested with real hardware, replace each
    // // Serial.println below with the actual actuator call (e.g. actuators.setRoomLed(...)).
    // if (buttons.wasLightButtonPressed())
    //     Serial.println("[Button] Light toggle pressed");
    // if (buttons.wasFanButtonPressed())
    //     Serial.println("[Button] Fan toggle pressed");
    // if (buttons.wasDoorButtonPressed())
    //     Serial.println("[Button] Door toggle pressed");
    // if (buttons.wasRoofButtonPressed())
    //     Serial.println("[Button] Roof toggle pressed");

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorRead = now;
        sensors.update();

        Serial.print("Temp: ");
        Serial.print(sensors.getTemperature());
        Serial.print(" | Gas: ");
        Serial.print(sensors.getGasValue());
        Serial.print(" | Rain: ");
        Serial.print(sensors.getRainRawValue());
        Serial.print(" | PIR: ");
        Serial.print(sensors.isMotionDetected() ? "Y" : "N");
        Serial.print(" | LDR: ");
        Serial.println(sensors.getLightLevel());
    }

    // DisplayManager throttles itself internally (DISPLAY_UPDATE_MS) - safe to call every loop
    display.update(sensors.getTemperature(), sensors.isRaining(), sensors.getGasValue(), sensors.isDark());
}