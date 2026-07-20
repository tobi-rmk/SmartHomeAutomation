#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "config.h"

// Owns the RC522 reader. Call begin() once, call update() every loop().
// Detects a new card, compares its UID against the stored authorized UID.
class RFIDManager
{
public:
    RFIDManager();

    void begin();

    // Call every loop(). Returns true exactly once when a NEW card was just scanned
    // (regardless of authorized or not) - use isLastCardAuthorized() to check the result.
    bool update();

    bool isLastCardAuthorized() const { return _lastAuthorized; }
    String getLastUID() const { return _lastUID; }

    // Call once (e.g. from a setup-time helper) to register the authorized UID,
    // or hardcode it directly in the .cpp if you prefer.
    void setAuthorizedUID(const byte *uid, byte length);

private:
    MFRC522 _rfid;
    byte _authorizedUID[10];
    byte _authorizedLength = 0;
    bool _lastAuthorized = false;
    String _lastUID = "";

    bool compareUID(const byte *uid, byte length) const;
};