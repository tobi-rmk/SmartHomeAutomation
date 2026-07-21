#pragma once

// ==================== LCD I2C ====================
#define LCD_ADDR 0x27 // If LCD shows nothing, run an I2C scanner to check the real address (could be 0x3F)
#define LCD_COLS 16
#define LCD_ROWS 2

// ==================== DHT11 ====================
#define DHT_PIN 4
#define DHT_TYPE DHT11

// ==================== MQ2 (gas) ====================
#define MQ2_PIN 34             // Analog pin, input-only
#define GAS_THRESHOLD_HIGH 750 // Above this -> enter gas alert state
#define GAS_THRESHOLD_LOW 700  // Below this -> return to normal state (hysteresis to avoid flicker)

// ==================== Rain sensor ====================
#define RAIN_PIN 35 // Analog pin, input-only
// LOWER analog value means WETTER (depends on module, must be calibrated via Serial Monitor)
#define RAIN_WET_THRESHOLD 3200 // Below this -> considered raining
#define RAIN_DRY_THRESHOLD 3400 // Above this -> considered dry (gap between the two = hysteresis, avoids flicker)

// ==================== PIR (motion detection) ====================
#define PIR_PIN 27

// ==================== LDR (light level) ====================
#define LDR_PIN 36 // Analog pin, input-only (VP)
// TODO: calibrate direction via Serial Monitor - some modules read HIGHER when darker, others LOWER.
// Values below assume HIGHER raw value = darker; flip the comparison in SensorManager if yours is inverted.
#define LIGHT_DARK_THRESHOLD 3000   // Above this -> considered dark
#define LIGHT_BRIGHT_THRESHOLD 2500 // Below this -> considered bright (gap = hysteresis, avoids flicker)

// ==================== Servos ====================
#define ROOF_SERVO_PIN 13 // Roof/awning servo
#define DOOR_SERVO_PIN 14 // Door servo

// ==================== Buzzer / LED / Fan ====================
#define BUZZER_PIN 25

#define LED_ROOM_PIN 26  // Room light, auto/manual control
#define LED_ALERT_PIN 33 // Alert light (e.g. gas alert)
#define FAN_RELAY_PIN 32 // Controls the fan via relay/transistor

// ==================== RFID RC522 (SPI) ====================
// SCK=18, MISO=19, MOSI=23 are ESP32's default SPI pins, no need to define separately
#define RFID_SS_PIN 5
#define RFID_RST_PIN 15

// ==================== RFID authorized card ====================
// Điền UID thẻ hợp lệ sau khi quét bằng code test (ví dụ: {0xDE, 0xAD, 0xBE, 0xEF})
#define AUTHORIZED_UID_LENGTH 4

// ==================== Button ====================

#define BTN_ROOF_PIN 16
#define BTN_DOOR_PIN 17
#define BTN_FAN_PIN 12

// ==================== Servo angles ====================
#define ROOF_CLOSED_ANGLE 90
#define ROOF_OPEN_ANGLE 0
#define DOOR_CLOSED_ANGLE 90
#define DOOR_OPEN_ANGLE 0

// ==================== Fan temperature thresholds ====================
#define FAN_TEMP_ON_THRESHOLD 30  // Above this + motion detected -> fan on
#define FAN_TEMP_OFF_THRESHOLD 28 // Below this -> fan off (regardless of motion)

// ==================== HomeController timing ====================
#define LIGHT_MOTION_HOLD_MS 15000        // Keep light on this long after the last motion, while still dark
#define WRONG_CARD_BEEP_MS 1500           // Buzzer pulse duration for an unauthorized RFID card
#define WRONG_CARD_ATTEMPTS_LIMIT 3       // Number of consecutive wrong scans before the alarm triggers
#define GAS_DOOR_AUTO_CLOSE_DELAY_MS 5000 // Auto-close the door this long after gas returns to normal

// ==================== Blynk ====================
#define BLYNK_SENSOR_PUSH_MS 2000 // How often to push Temp/Gas values to the dashboard

// ==================== Timing ====================
#define WARMUP_SECONDS 30
#define SENSOR_READ_INTERVAL_MS 1000 // Read sensors every 1s
#define DISPLAY_UPDATE_MS 1000       // Update LCD every 1s
#define DEBOUNCE_DELAY_MS 50         // Button