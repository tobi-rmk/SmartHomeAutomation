#pragma once

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

// Owns all outputs: 2 servo (roof, door), buzzer, 2 LED, fan relay.
// Call begin() once in setup(). Setter methods are idempotent - safe to call every loop.
class ActuatorManager
{
public:
    void begin();

    void setRoofOpen(bool open);
    void setDoorOpen(bool open);
    void setBuzzer(bool on);
    void setRoomLed(bool on);
    void setAlertLed(bool on);
    void setFan(bool on);

    bool isRoofOpen() const { return _roofOpen; }
    bool isDoorOpen() const { return _doorOpen; }
    bool isBuzzerOn() const { return _buzzerOn; }
    bool isRoomLedOn() const { return _roomLedOn; }
    bool isAlertLedOn() const { return _alertLedOn; }
    bool isFanOn() const { return _fanOn; }

private:
    Servo _roofServo;
    Servo _doorServo;

    bool _roofOpen = false;
    bool _doorOpen = false;
    bool _buzzerOn = false;
    bool _roomLedOn = false;
    bool _alertLedOn = false;
    bool _fanOn = false;
};