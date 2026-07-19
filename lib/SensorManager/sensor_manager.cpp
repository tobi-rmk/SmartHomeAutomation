#include "sensor_manager.h"
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

    int rainRaw = analogRead(RAIN_PIN);
    _isRaining = (rainRaw < RAIN_THRESHOLD);

    _motionDetected = (digitalRead(PIR_PIN) == HIGH);

    _lightLevel = analogRead(LDR_PIN);
}