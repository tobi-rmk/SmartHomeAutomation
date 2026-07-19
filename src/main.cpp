#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"
#include "sensor_manager.h"

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
SensorManager sensors;

enum DisplayState
{
    STATE_NORMAL,
    STATE_GAS_ALERT
};
DisplayState currentState = STATE_NORMAL;

unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;

// ---------------- Warm-up: countdown shown on LCD ----------------
void runWarmup()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warming up...");

    unsigned long warmupStart = millis();
    int secondsLeft = WARMUP_SECONDS;
    int lastShown = -1;

    while (secondsLeft > 0)
    {
        unsigned long elapsed = millis() - warmupStart;
        secondsLeft = WARMUP_SECONDS - (int)(elapsed / 1000);
        if (secondsLeft < 0)
            secondsLeft = 0;

        if (secondsLeft != lastShown)
        {
            lcd.setCursor(0, 1);
            lcd.print("Remaining: ");
            lcd.print(secondsLeft);
            lcd.print("s  "); // trailing spaces clear leftover chars from a longer previous value
            lastShown = secondsLeft;
        }
    }

    lcd.clear();
}

// ---------------- State transition (hysteresis to avoid flicker) ----------------
void updateState()
{
    int gas = sensors.getGasValue();

    if (currentState == STATE_NORMAL && gas > GAS_THRESHOLD_HIGH)
    {
        currentState = STATE_GAS_ALERT;
        lcd.clear();
    }
    else if (currentState == STATE_GAS_ALERT && gas < GAS_THRESHOLD_LOW)
    {
        currentState = STATE_NORMAL;
        lcd.clear();
    }
}

// ---------------- Render LCD based on current state ----------------
void updateDisplay()
{
    lcd.setCursor(0, 0);

    if (currentState == STATE_NORMAL)
    {
        lcd.print("Temp: ");
        lcd.print(sensors.getTemperature(), 1);
        lcd.print(" C   ");

        lcd.setCursor(0, 1);
        lcd.print("Rain: ");
        lcd.print(sensors.isRaining() ? "Y  " : "N  ");
    }
    else
    {
        lcd.print("!! GAS ALERT !!");
        lcd.setCursor(0, 1);
        lcd.print("Gas: ");
        lcd.print(sensors.getGasValue());
        lcd.print("      ");
    }
}

void setup()
{
    Serial.begin(115200);

    Wire.begin();
    lcd.init();
    lcd.backlight();

    sensors.begin();

    runWarmup();
}

void loop()
{
    unsigned long now = millis();

    if (now - lastSensorRead >= SENSOR_READ_INTERVAL_MS)
    {
        lastSensorRead = now;
        sensors.update();
        updateState();

        Serial.print("Temp: ");
        Serial.print(sensors.getTemperature());
        Serial.print(" | Gas: ");
        Serial.print(sensors.getGasValue());
        Serial.print(" | Rain: ");
        Serial.print(sensors.isRaining() ? "Y" : "N");
        Serial.print(" | PIR: ");
        Serial.print(sensors.isMotionDetected() ? "Y" : "N");
        Serial.print(" | LDR: ");
        Serial.println(sensors.getLightLevel());
    }

    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_MS)
    {
        lastDisplayUpdate = now;
        updateDisplay();
    }
}