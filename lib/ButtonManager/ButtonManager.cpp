#include "ButtonManager.h"

void ButtonManager::begin()
{
    pinMode(_roof.pin, INPUT_PULLUP);
    pinMode(_door.pin, INPUT_PULLUP);
    pinMode(_fan.pin, INPUT_PULLUP);
}

void ButtonManager::updateOne(ButtonState &b)
{
    bool raw = digitalRead(b.pin);

    if (raw != b.lastRaw)
    {
        b.lastChangeTime = millis();
    }

    if ((millis() - b.lastChangeTime) > DEBOUNCE_DELAY_MS)
    {
        if (raw != b.stableState)
        {
            b.stableState = raw;
            if (b.stableState == LOW)
            {
                b.pressedEvent = true;
            }
        }
    }

    b.lastRaw = raw;
}

bool ButtonManager::consumeEvent(ButtonState &b)
{
    if (b.pressedEvent)
    {
        b.pressedEvent = false;
        return true;
    }
    return false;
}

void ButtonManager::update()
{
    updateOne(_roof);
    updateOne(_door);
    updateOne(_fan);
}

bool ButtonManager::wasRoofPressed() { return consumeEvent(_roof); }
bool ButtonManager::wasDoorPressed() { return consumeEvent(_door); }
bool ButtonManager::wasFanPressed() { return consumeEvent(_fan); }