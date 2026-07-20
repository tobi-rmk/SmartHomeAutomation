#include <Arduino.h>
#include "config.h"
#include "RFIDManager.h"

RFIDManager rfid;

// UID thẻ hợp lệ - thay bằng UID thật của bạn sau khi quẹt lần đầu và đọc từ Serial
byte authorizedUID[] = {0x10, 0xCC, 0xD6, 0x0B};

void setup()
{
    Serial.begin(115200);
    rfid.begin();
    rfid.setAuthorizedUID(authorizedUID, sizeof(authorizedUID));
    Serial.println("RFID ready. Scan a card...");
}

void loop()
{
    if (rfid.update())
    {
        Serial.print("UID: ");
        Serial.print(rfid.getLastUID());
        Serial.print(" -> ");
        Serial.println(rfid.isLastCardAuthorized() ? "ACCESS GRANTED" : "ACCESS DENIED");
    }
}