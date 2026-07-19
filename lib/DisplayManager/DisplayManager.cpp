#include <Wire.h>
#include "DisplayManager.h"

DisplayManager::DisplayManager() : _lcd(LCD_ADDR, LCD_COLS, LCD_ROWS) {}

void DisplayManager::begin()
{
    Wire.begin();
    _lcd.init();
    _lcd.backlight();
}

void DisplayManager::runWarmup(int seconds)
{
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print("Warming up...");

    unsigned long warmupStart = millis();
    int secondsLeft = seconds;
    int lastShown = -1;

    while (secondsLeft > 0)
    {
        unsigned long elapsed = millis() - warmupStart;
        secondsLeft = seconds - (int)(elapsed / 1000);
        if (secondsLeft < 0)
            secondsLeft = 0;

        if (secondsLeft != lastShown)
        {
            _lcd.setCursor(0, 1);
            _lcd.print("Remaining: ");
            _lcd.print(secondsLeft);
            _lcd.print("s  "); // trailing spaces clear leftover chars from a longer previous value
            lastShown = secondsLeft;
        }
    }

    _lcd.clear();
}

void DisplayManager::update(float temperature, bool isRaining, int gasValue, bool isDark)
{
    unsigned long now = millis();
    if (now - _lastUpdate < DISPLAY_UPDATE_MS)
        return;
    _lastUpdate = now;

    // State transition with hysteresis (two thresholds) to avoid flicker at the boundary
    if (_state == STATE_NORMAL && gasValue > GAS_THRESHOLD_HIGH)
    {
        _state = STATE_GAS_ALERT;
        _lcd.clear();
    }
    else if (_state == STATE_GAS_ALERT && gasValue < GAS_THRESHOLD_LOW)
    {
        _state = STATE_NORMAL;
        _lcd.clear();
    }

    render(temperature, isRaining, gasValue, isDark);
}

void DisplayManager::render(float temperature, bool isRaining, int gasValue, bool isDark)
{
    _lcd.setCursor(0, 0);

    if (_state == STATE_NORMAL)
    {
        // Line 1: temperature + gas value (short labels, LCD is only 16 columns wide)
        _lcd.print("T:");
        _lcd.print(temperature, 1);
        _lcd.print((char)223);
        _lcd.setCursor(9, 0);
        _lcd.print("G:");
        _lcd.print(gasValue);
        _lcd.print("   ");

        // Line 2: rain + light status
        _lcd.setCursor(0, 1);
        _lcd.print("R:");
        _lcd.print(isRaining ? "Rain " : "Dry ");
        _lcd.print("L:");
        _lcd.print(isDark ? "Dark  " : "Bright");
    }
    else
    {
        _lcd.print("!! GAS ALERT !!");
        _lcd.setCursor(0, 1);
        _lcd.print("Gas: ");
        _lcd.print(gasValue);
        _lcd.print("      ");
    }
}