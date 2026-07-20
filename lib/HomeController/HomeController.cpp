#include "HomeController.h"
#include "config.h"

void HomeController::begin()
{
    // Nothing to initialize - all managers own their own pins/hardware and are begin()'d
    // separately in main.cpp. HomeController only holds orchestration state.
}

void HomeController::update(SensorManager &sensors, ActuatorManager &actuators,
                            ButtonManager &buttons, RFIDManager &rfid)
{
    handleRoof(sensors, actuators, buttons);
    handleDoorAndRFID(sensors, actuators, buttons, rfid);
    handleFan(sensors, actuators, buttons);
    handleGasAlarm(sensors, actuators);
    handleGasDoorAutoClose(actuators);
    handleLight(sensors, actuators);
    resolveAlarmOutputs(actuators);
}

// ==================== Logic 2: Rain -> roof, button overrides ====================
void HomeController::handleRoof(SensorManager &sensors, ActuatorManager &actuators, ButtonManager &buttons)
{
    bool raining = sensors.isRaining();

    if (raining && !_wasRaining)
    {
        // Rain just started -> open/extend the roof to cover
        actuators.setRoofOpen(true);
        Serial.println("[Auto] Rain started -> roof opened (covering)");
    }
    else if (!raining && _wasRaining)
    {
        // Rain just stopped -> close/retract the roof
        actuators.setRoofOpen(false);
        Serial.println("[Auto] Rain stopped -> roof closed (retracted)");
    }
    _wasRaining = raining;

    // Manual override: always allowed, independent of the automatic rule above.
    // It only gets overwritten again on the NEXT rain edge, not on every loop.
    if (buttons.wasRoofPressed())
    {
        actuators.setRoofOpen(!actuators.isRoofOpen());
        Serial.print("[Button] Roof -> ");
        Serial.println(actuators.isRoofOpen() ? "OPEN" : "CLOSED");
    }
}

// ==================== Logic 3 + 5: RFID / door / wrong-card beep ====================
void HomeController::handleDoorAndRFID(SensorManager &sensors, ActuatorManager &actuators,
                                       ButtonManager &buttons, RFIDManager &rfid)
{
    if (rfid.update())
    {
        if (rfid.isLastCardAuthorized())
        {
            actuators.setDoorOpen(true);
            _gasDoorCloseScheduled = false; // cancel any pending gas auto-close - door just opened legitimately
            _wrongCardCount = 0;            // a successful scan resets the wrong-attempt counter
            Serial.print("[RFID] Authorized card ");
            Serial.print(rfid.getLastUID());
            Serial.println(" -> door opened");
        }
        else
        {
            _wrongCardCount++;
            Serial.print("[RFID] DENIED card ");
            Serial.print(rfid.getLastUID());
            Serial.print(" (attempt ");
            Serial.print(_wrongCardCount);
            Serial.print("/");
            Serial.print(WRONG_CARD_ATTEMPTS_LIMIT);
            Serial.println(")");

            if (_wrongCardCount >= WRONG_CARD_ATTEMPTS_LIMIT)
            {
                _wrongCardBeepActive = true;
                _wrongCardBeepStarted = millis();
                _wrongCardCount = 0; // reset so the alarm only fires once per batch of wrong attempts
                Serial.println("[RFID] 3 wrong attempts -> ALARM");
            }
        }
    }

    // Wrong-card beep is a timed pulse, not a held state - clear it once WRONG_CARD_BEEP_MS has passed
    if (_wrongCardBeepActive && (millis() - _wrongCardBeepStarted >= WRONG_CARD_BEEP_MS))
    {
        _wrongCardBeepActive = false;
    }

    // Manual override: open/close the door from inside at any time
    if (buttons.wasDoorPressed())
    {
        actuators.setDoorOpen(!actuators.isDoorOpen());
        _gasDoorCloseScheduled = false; // manual action always cancels the pending auto-close
        Serial.print("[Button] Door -> ");
        Serial.println(actuators.isDoorOpen() ? "OPEN" : "CLOSED");
    }
}

// ==================== Logic 4: Temperature -> fan, button overrides ====================
void HomeController::handleFan(SensorManager &sensors, ActuatorManager &actuators, ButtonManager &buttons)
{
    float temp = sensors.getTemperature();

    // shouldBeOn tracks ONLY the temperature condition. It is never influenced
    // by what the actuator/button did - this is what stops it from getting "stuck" after
    // a manual override happens while the condition was already true.
    bool shouldBeOn = _tempAboveThreshold;
    if (!shouldBeOn && temp > FAN_TEMP_ON_THRESHOLD)
    {
        shouldBeOn = true;
    }
    else if (shouldBeOn && temp < FAN_TEMP_OFF_THRESHOLD)
    {
        shouldBeOn = false;
    }

    if (shouldBeOn != _tempAboveThreshold)
    {
        _tempAboveThreshold = shouldBeOn;
        actuators.setFan(shouldBeOn);
        Serial.println(shouldBeOn ? "[Auto] Temp above threshold -> fan on" : "[Auto] Cooled down -> fan off");
    }

    // Manual override: always allowed, and does NOT touch _tempAboveThreshold at all.
    // It only gets re-asserted by the automatic rule above on the NEXT real condition edge.
    if (buttons.wasFanPressed())
    {
        actuators.setFan(!actuators.isFanOn());
        Serial.print("[Button] Fan -> ");
        Serial.println(actuators.isFanOn() ? "ON" : "OFF");
    }
}

// ==================== Logic 5: Gas -> buzzer + auto-open door ====================
void HomeController::handleGasAlarm(SensorManager &sensors, ActuatorManager &actuators)
{
    int gas = sensors.getGasValue();

    if (!_gasAlarmActive && gas > GAS_THRESHOLD_HIGH)
    {
        _gasAlarmActive = true;
        _gasDoorCloseScheduled = false; // cancel any pending auto-close - gas is high again
        actuators.setDoorOpen(true);
        Serial.println("[Auto] GAS ALERT -> buzzer on, door opened");
    }
    else if (_gasAlarmActive && gas < GAS_THRESHOLD_LOW)
    {
        _gasAlarmActive = false;
        _gasDoorCloseScheduled = true;
        _gasDoorCloseAt = millis() + GAS_DOOR_AUTO_CLOSE_DELAY_MS;
        Serial.println("[Auto] Gas back to normal -> buzzer off, door will auto-close in 5s");
    }
}

void HomeController::handleGasDoorAutoClose(ActuatorManager &actuators)
{
    if (_gasDoorCloseScheduled && millis() >= _gasDoorCloseAt)
    {
        _gasDoorCloseScheduled = false;
        actuators.setDoorOpen(false);
        Serial.println("[Auto] Door auto-closed after gas alarm cleared");
    }
}

// ==================== Logic 6: Darkness + motion -> light, with hold timer ====================
void HomeController::handleLight(SensorManager &sensors, ActuatorManager &actuators)
{
    if (!sensors.isDark())
    {
        // Bright enough - light never needs to be on
        actuators.setRoomLed(false);
        return;
    }

    // It's dark from here on
    if (sensors.isMotionDetected())
    {
        _lastMotionWhileDark = millis();
        actuators.setRoomLed(true);
        return;
    }

    // Dark, no motion right now - keep the light on until the hold timer expires
    if (millis() - _lastMotionWhileDark > LIGHT_MOTION_HOLD_MS)
    {
        actuators.setRoomLed(false);
    }
    // else: still within the hold window - leave the light as-is (on)
}

// ==================== Alarm outputs resolver: gas alarm OR wrong-card beep ====================
void HomeController::resolveAlarmOutputs(ActuatorManager &actuators)
{
    bool shouldAlarm = _gasAlarmActive || _wrongCardBeepActive;

    if (shouldAlarm != actuators.isBuzzerOn())
    {
        actuators.setBuzzer(shouldAlarm);
    }
    if (shouldAlarm != actuators.isAlertLedOn())
    {
        actuators.setAlertLed(shouldAlarm);
    }
}