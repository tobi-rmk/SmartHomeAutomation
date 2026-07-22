#include "HomeController.h"
#include "config.h"

void HomeController::begin()
{
    // Nothing to initialize - all managers own their own pins/hardware and are begin()'d
    // separately in main.cpp. HomeController only holds orchestration state.
}

void HomeController::update(SensorManager &sensors, ActuatorManager &actuators,
                            ButtonManager &buttons, RFIDManager &rfid, BlynkManager &blynk)
{
    handleRoof(sensors, actuators, buttons, blynk);
    handleDoorAndRFID(sensors, actuators, buttons, rfid, blynk);
    handleFan(sensors, actuators, buttons, blynk);
    handleGasAlarm(sensors, actuators);
    handleGasDoorAutoClose(actuators);
    handleLight(sensors, actuators);
    resolveAlarmOutputs(actuators);
    updateBlynk(sensors, blynk);
}

// ==================== Logic 2: Rain -> roof, button/app overrides ====================
void HomeController::handleRoof(SensorManager &sensors, ActuatorManager &actuators,
                                ButtonManager &buttons, BlynkManager &blynk)
{
    bool raining = sensors.isRaining();
    bool changed = false;

    if (raining && !_wasRaining)
    {
        // Rain just started -> open/extend the roof to cover
        actuators.setRoofOpen(true);
        changed = true;
        Serial.println("[Auto] Rain started -> roof opened (covering)");
    }
    else if (!raining && _wasRaining)
    {
        // Rain just stopped -> close/retract the roof
        actuators.setRoofOpen(false);
        changed = true;
        Serial.println("[Auto] Rain stopped -> roof closed (retracted)");
    }
    _wasRaining = raining;

    // Manual override from the physical button: always allowed, independent of the
    // automatic rule above. Only gets overwritten again on the NEXT rain edge.
    if (buttons.wasRoofPressed())
    {
        actuators.setRoofOpen(!actuators.isRoofOpen());
        changed = true;
        Serial.print("[Button] Roof -> ");
        Serial.println(actuators.isRoofOpen() ? "OPEN" : "CLOSED");
    }

    // Manual override from the Blynk app switch - same treatment, just sets the
    // exact requested state instead of toggling.
    bool appValue;
    if (blynk.consumeRoofCommand(appValue))
    {
        actuators.setRoofOpen(appValue);
        changed = true;
        Serial.print("[Blynk] Roof -> ");
        Serial.println(actuators.isRoofOpen() ? "OPEN" : "CLOSED");
    }

    // Push the (possibly new) state back to the dashboard so the switch widget
    // reflects reality regardless of which source changed it.
    if (changed)
        blynk.syncRoof(actuators.isRoofOpen());
}

// ==================== Logic 3 + 5: RFID / door / wrong-card beep ====================
void HomeController::handleDoorAndRFID(SensorManager &sensors, ActuatorManager &actuators,
                                       ButtonManager &buttons, RFIDManager &rfid, BlynkManager &blynk)
{
    bool changed = false;

    if (rfid.update())
    {
        if (rfid.isLastCardAuthorized())
        {
            actuators.setDoorOpen(true);
            changed = true;
            _gasDoorCloseScheduled = false; // cancel any pending gas auto-close - door just opened legitimately
            _wrongCardCount = 0;            // a successful scan resets the wrong-attempt counter
            startBeep(1, 150, 0);           // single short chirp = success
            Serial.print("[RFID] Authorized card ");
            Serial.print(rfid.getLastUID());
            Serial.println(" -> door opened");
        }
        else
        {
            _wrongCardCount++;
            startBeep(2, 80, 80); // double quick chirp = wrong card (not yet at the alarm limit)
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
        changed = true;
        _gasDoorCloseScheduled = false; // manual action always cancels the pending auto-close
        Serial.print("[Button] Door -> ");
        Serial.println(actuators.isDoorOpen() ? "OPEN" : "CLOSED");
    }

    // Manual override from the Blynk app switch
    bool appValue;
    if (blynk.consumeDoorCommand(appValue))
    {
        actuators.setDoorOpen(appValue);
        changed = true;
        _gasDoorCloseScheduled = false;
        Serial.print("[Blynk] Door -> ");
        Serial.println(actuators.isDoorOpen() ? "OPEN" : "CLOSED");
    }

    if (changed)
        blynk.syncDoor(actuators.isDoorOpen());

    // door_alert event: fires once when the 3-wrong-attempt alarm activates
    blynk.notifyDoorAlert(_wrongCardBeepActive);
}

// ==================== Logic 4: Temperature + motion -> fan, button/app overrides ====================
void HomeController::handleFan(SensorManager &sensors, ActuatorManager &actuators,
                               ButtonManager &buttons, BlynkManager &blynk)
{
    float temp = sensors.getTemperature();
    bool changed = false;

    // "hot" tracks ONLY the temperature condition (hysteresis), independent of motion/actuator/button.
    bool hot = _tempAboveThreshold;
    if (!hot && temp > FAN_TEMP_ON_THRESHOLD)
    {
        hot = true;
    }
    else if (hot && temp < FAN_TEMP_OFF_THRESHOLD)
    {
        hot = false;
    }
    _tempAboveThreshold = hot;

    if (sensors.isMotionDetected())
    {
        _lastMotionWhileHot = millis();
    }

    // Fan should spin only when hot AND (motion right now OR within the hold window since
    // the last motion) - exactly the same pattern as the light's hold timer.
    bool desiredAutoOn = hot && (sensors.isMotionDetected() ||
                                 (millis() - _lastMotionWhileHot <= FAN_MOTION_HOLD_MS));

    // Edge-triggered: only touch the actuator when the AUTOMATIC decision itself changes,
    // never every loop - this is what keeps manual overrides from being silently overwritten.
    if (desiredAutoOn != _fanAutoOn)
    {
        _fanAutoOn = desiredAutoOn;
        actuators.setFan(desiredAutoOn);
        changed = true;
        Serial.println(desiredAutoOn ? "[Auto] Hot + motion -> fan on" : "[Auto] Cooled down or no motion -> fan off");
    }

    // Manual override: always allowed, does NOT touch _fanAutoOn at all.
    if (buttons.wasFanPressed())
    {
        actuators.setFan(!actuators.isFanOn());
        changed = true;
        Serial.print("[Button] Fan -> ");
        Serial.println(actuators.isFanOn() ? "ON" : "OFF");
    }

    bool appValue;
    if (blynk.consumeFanCommand(appValue))
    {
        actuators.setFan(appValue);
        changed = true;
        Serial.print("[Blynk] Fan -> ");
        Serial.println(actuators.isFanOn() ? "ON" : "OFF");
    }

    if (changed)
        blynk.syncFan(actuators.isFanOn());
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

// ==================== Alarm outputs resolver: gas alarm OR wrong-card alarm OR transient beep ====================
void HomeController::resolveAlarmOutputs(ActuatorManager &actuators)
{
    bool realAlarm = _gasAlarmActive || _wrongCardBeepActive;
    bool buzzerOn = realAlarm || isBeepCurrentlyOn();

    if (buzzerOn != actuators.isBuzzerOn())
    {
        actuators.setBuzzer(buzzerOn);
    }
    // Alert LED only reflects a REAL alarm (gas/3-strike) - not routine success/error chirps,
    // otherwise the alert LED would flash on every single successful door scan.
    if (realAlarm != actuators.isAlertLedOn())
    {
        actuators.setAlertLed(realAlarm);
    }
}

// ==================== Non-blocking beep pattern player ====================
void HomeController::startBeep(int count, unsigned long onMs, unsigned long offMs)
{
    _beepActive = true;
    _beepStartedAt = millis();
    _beepCount = count;
    _beepOnMs = onMs;
    _beepOffMs = offMs;
}

bool HomeController::isBeepCurrentlyOn()
{
    if (!_beepActive)
        return false;

    unsigned long elapsed = millis() - _beepStartedAt;
    unsigned long cycle = _beepOnMs + _beepOffMs;
    unsigned long totalDuration = (unsigned long)_beepCount * cycle;

    if (elapsed >= totalDuration)
    {
        _beepActive = false; // pattern finished
        return false;
    }

    unsigned long posInCycle = elapsed % cycle;
    return posInCycle < _beepOnMs;
}

// ==================== Push Temp/Gas/Motion/Rain + gas_alert event to Blynk ====================
void HomeController::updateBlynk(SensorManager &sensors, BlynkManager &blynk)
{
    blynk.updateSensors(sensors.getTemperature(), sensors.getGasValue(),
                        sensors.isMotionDetected(), sensors.isRaining());
    blynk.notifyGasAlert(_gasAlarmActive);
}