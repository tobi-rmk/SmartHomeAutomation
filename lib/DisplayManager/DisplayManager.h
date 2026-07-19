#pragma once

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

enum DisplayState
{
    STATE_NORMAL,
    STATE_GAS_ALERT
};

// Owns the LCD. Handles: warmup countdown, and switching between
// state 1 (Temp/Rain) <-> state 2 (Gas alert) with hysteresis.
// Call begin() once in setup(), runWarmup() once after begin(),
// then call update(...) every loop() iteration - it throttles itself internally.
class DisplayManager
{
public:
    DisplayManager();

    void begin();
    void runWarmup(int seconds = WARMUP_SECONDS);

    // Safe to call every loop() - only actually refreshes the LCD every DISPLAY_UPDATE_MS
    void update(float temperature, bool isRaining, int gasValue, bool isDark);

private:
    LiquidCrystal_I2C _lcd;
    DisplayState _state = STATE_NORMAL;
    unsigned long _lastUpdate = 0;

    void render(float temperature, bool isRaining, int gasValue, bool isDark);
};