#include "RFIDManager.h"

RFIDManager::RFIDManager() : _rfid(RFID_SS_PIN, RFID_RST_PIN) {}

void RFIDManager::begin()
{
    SPI.begin();
    _rfid.PCD_Init();
}

void RFIDManager::setAuthorizedUID(const byte *uid, byte length)
{
    _authorizedLength = length;
    for (byte i = 0; i < length && i < sizeof(_authorizedUID); i++)
    {
        _authorizedUID[i] = uid[i];
    }
}

bool RFIDManager::compareUID(const byte *uid, byte length) const
{
    if (length != _authorizedLength)
        return false;
    for (byte i = 0; i < length; i++)
    {
        if (uid[i] != _authorizedUID[i])
            return false;
    }
    return true;
}

bool RFIDManager::update()
{
    if (!_rfid.PICC_IsNewCardPresent() || !_rfid.PICC_ReadCardSerial())
        return false;

    // Build a readable UID string, e.g. "DE AD BE EF"
    String uidStr = "";
    for (byte i = 0; i < _rfid.uid.size; i++)
    {
        if (_rfid.uid.uidByte[i] < 0x10)
            uidStr += "0";
        uidStr += String(_rfid.uid.uidByte[i], HEX);
        if (i < _rfid.uid.size - 1)
            uidStr += " ";
    }
    uidStr.toUpperCase();
    _lastUID = uidStr;

    _lastAuthorized = compareUID(_rfid.uid.uidByte, _rfid.uid.size);

    _rfid.PICC_HaltA();
    _rfid.PCD_StopCrypto1();

    return true;
}