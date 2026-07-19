#pragma once

#include <Arduino.h>
#include <DHT.h>

// Reads and stores the state of: DHT11, MQ2, Rain, PIR, LDR
// Call begin() once in setup(), call update() periodically in loop()
class SensorManager
{
public:
    SensorManager();

    void begin();
    void update();

    float getTemperature() const { return _temperature; }
    float getHumidity() const { return _humidity; }
    int getGasValue() const { return _gasValue; }
    bool isRaining() const { return _isRaining; }
    bool isMotionDetected() const { return _motionDetected; }
    int getLightLevel() const { return _lightLevel; }

private:
    DHT _dht;

    float _temperature = 0;
    float _humidity = 0;
    int _gasValue = 0;
    bool _isRaining = false;
    bool _motionDetected = false;
    int _lightLevel = 0;
};