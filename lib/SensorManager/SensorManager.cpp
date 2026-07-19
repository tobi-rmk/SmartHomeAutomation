#include "SensorManager.h"
#include "config.h"

SensorManager::SensorManager() : _dht(DHT_PIN, DHT_TYPE) {}

void SensorManager::begin()
{
    _dht.begin();
    pinMode(MQ2_PIN, INPUT);
    pinMode(RAIN_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    // LDR_PIN is analog, no pinMode needed
}

void SensorManager::update()
{
    float t = _dht.readTemperature();
    float h = _dht.readHumidity();
    // DHT11 occasionally fails and returns NaN -> keep the previous value instead of overwriting with NaN
    if (!isnan(t))
        _temperature = t;
    if (!isnan(h))
        _humidity = h;

    _gasValue = analogRead(MQ2_PIN);

    // Rain: hysteresis with two thresholds to avoid flicker at the boundary
    _rainRaw = analogRead(RAIN_PIN);
    if (!_isRaining && _rainRaw < RAIN_WET_THRESHOLD)
    {
        _isRaining = true;
    }
    else if (_isRaining && _rainRaw > RAIN_DRY_THRESHOLD)
    {
        _isRaining = false;
    }

    _motionDetected = (digitalRead(PIR_PIN) == HIGH);

    // Light: same hysteresis approach
    _lightLevel = analogRead(LDR_PIN);
    if (!_isDark && _lightLevel > LIGHT_DARK_THRESHOLD)
    {
        _isDark = true;
    }
    else if (_isDark && _lightLevel < LIGHT_BRIGHT_THRESHOLD)
    {
        _isDark = false;
    }
}