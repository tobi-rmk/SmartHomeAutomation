#include "ActuatorManager.h"

void ActuatorManager::begin()
{
    _roofServo.setPeriodHertz(50);
    _doorServo.setPeriodHertz(50);
    _roofServo.attach(ROOF_SERVO_PIN, 500, 2400);
    _doorServo.attach(DOOR_SERVO_PIN, 500, 2400);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_ROOM_PIN, OUTPUT);
    pinMode(LED_ALERT_PIN, OUTPUT);
    pinMode(FAN_RELAY_PIN, OUTPUT);

    setRoofOpen(false);
    setDoorOpen(false);
    setBuzzer(false);
    setRoomLed(false);
    setAlertLed(false);
    setFan(false);
}

void ActuatorManager::setRoofOpen(bool open)
{
    _roofOpen = open;
    _roofServo.write(open ? ROOF_OPEN_ANGLE : ROOF_CLOSED_ANGLE);
}

void ActuatorManager::setDoorOpen(bool open)
{
    _doorOpen = open;
    _doorServo.write(open ? DOOR_OPEN_ANGLE : DOOR_CLOSED_ANGLE);
}

void ActuatorManager::setBuzzer(bool on)
{
    _buzzerOn = on;
    digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
}

void ActuatorManager::setRoomLed(bool on)
{
    _roomLedOn = on;
    digitalWrite(LED_ROOM_PIN, on ? HIGH : LOW);
}

void ActuatorManager::setAlertLed(bool on)
{
    _alertLedOn = on;
    digitalWrite(LED_ALERT_PIN, on ? HIGH : LOW);
}

void ActuatorManager::setFan(bool on)
{
    _fanOn = on;
    digitalWrite(FAN_RELAY_PIN, on ? LOW : HIGH); // Active LOW: LOW = đóng relay = bật quạt
}