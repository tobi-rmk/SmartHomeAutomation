#pragma once

#include <Arduino.h>
#include "config.h"

// Reads 3 buttons with debounce, exposes edge-detected "was pressed" (one-shot per press).
// Call begin() once, call update() every loop() before checking wasXPressed().
class ButtonManager
{
public:
    void begin();
    void update();

    bool wasRoofPressed();
    bool wasDoorPressed();
    bool wasFanPressed();

private:
    struct ButtonState
    {
        uint8_t pin;
        bool lastRaw = HIGH;
        bool stableState = HIGH;
        unsigned long lastChangeTime = 0;
        bool pressedEvent = false;

        ButtonState(uint8_t p) : pin(p) {}
    };

    ButtonState _roof{BTN_ROOF_PIN};
    ButtonState _door{BTN_DOOR_PIN};
    ButtonState _fan{BTN_FAN_PIN};

    void updateOne(ButtonState &b);
    bool consumeEvent(ButtonState &b);
};