#include <Arduino.h>
#include "config.h"
#include "ActuatorManager.h"
#include "ButtonManager.h"

ActuatorManager actuators;
ButtonManager buttons;

void setup()
{
    Serial.begin(115200);
    actuators.begin();
    buttons.begin();

    Serial.println("Stage B test ready. Press buttons to toggle actuators.");

    actuators.setBuzzer(true);
    actuators.setAlertLed(true);
    actuators.setRoomLed(true);
    delay(3000);
    actuators.setBuzzer(false);
    actuators.setAlertLed(false);
    actuators.setRoomLed(false);
}

void loop()
{
    buttons.update();

    if (buttons.wasRoofPressed())
    {
        actuators.setRoofOpen(!actuators.isRoofOpen());
        Serial.print("Roof: ");
        Serial.println(actuators.isRoofOpen() ? "OPEN" : "CLOSED");
    }
    if (buttons.wasDoorPressed())
    {
        actuators.setDoorOpen(!actuators.isDoorOpen());
        Serial.print("Door: ");
        Serial.println(actuators.isDoorOpen() ? "OPEN" : "CLOSED");
    }
    if (buttons.wasFanPressed())
    {
        actuators.setFan(!actuators.isFanOn());
        Serial.print("Fan: ");
        Serial.println(actuators.isFanOn() ? "ON" : "OFF");
    }
}