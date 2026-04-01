// LoRaWAN Moisture + Temperature Node — WITH OLED DISPLAY
// Target hardware: TTGO LoRa32 (ESP32 + SX1276 + SSD1306 OLED)
// Framework: Arduino
//
// NOTE: This version is a work in progress — OLED causes boot crash when
// combined with LMIC. See GitHub issue / debug notes.
//
// Required libraries (install via Arduino Library Manager):
//   - MCCI LoRaWAN LMIC library
//   - DHT sensor library (Adafruit)
//   - Adafruit Unified Sensor
//   - ESP8266 and ESP32 OLED driver for SSD1306 displays (ThingPulse)

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <DHT.h>

// ---------- OLED config ----------
#define OLED_SDA    21
#define OLED_SCL    22

SSD1306Wire* oled = nullptr;

// ---------- Sensor config ----------
#define DHT_PIN     13    // moved from 22 (conflicts with OLED SCL)
#define DHT_TYPE    DHT22
#define MOISTURE_PIN 34

DHT dht(DHT_PIN, DHT_TYPE);

// ---------- LoRaWAN credentials ----------
static const u1_t PROGMEM APPEUI[8]  = { 0x83, 0xA3, 0x2C, 0x4F, 0xFC, 0xED, 0x4B, 0x89 }; // 894bedfc4f2ca383 (LSB)
static const u1_t PROGMEM DEVEUI[8]  = { 0xFF, 0xFE, 0x14, 0x05, 0x94, 0xF9, 0x24, 0xF0 }; // F024F994 0514FEFF (LSB)
static const u1_t PROGMEM APPKEY[16] = { 0x4F, 0x3F, 0xA4, 0x78, 0x70, 0x2A, 0xF1, 0x9C,
                                          0x1B, 0xC5, 0xD0, 0x40, 0x63, 0x56, 0x8A, 0x20 };

void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

// ---------- LMIC pin mapping for TTGO LoRa32 ----------
const lmic_pinmap lmic_pins = {
    .nss  = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst  = 14,
    .dio  = {26, 33, 32},
};

// ---------- State ----------
static osjob_t sendjob;
const unsigned TX_INTERVAL = 60;

bool  joined        = false;
unsigned long lastTxMillis = 0;
int   lastCountdown = -1;

// ---------- OLED helpers ----------
void drawDisplay(const char* status, int countdown) {
    if (!oled) return;
    oled->clear();

    oled->setFont(ArialMT_Plain_10);
    oled->drawString(0, 0, "LoRaWAN Node");
    oled->drawLine(0, 12, 128, 12);

    oled->drawString(0, 15, String("Status: ") + status);

    if (countdown >= 0) {
        oled->drawString(0, 30, String("Next TX: ") + countdown + "s");

        // Progress bar (fills as countdown approaches 0)
        int barW = map(countdown, TX_INTERVAL, 0, 0, 124);
        oled->drawRect(0, 48, 128, 12);
        oled->fillRect(2, 50, barW, 8);
    } else {
        oled->drawString(0, 30, "Next TX: --");
    }

    oled->display();
}

// ---------- Read sensors and transmit ----------
void do_send(osjob_t* j) {
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println("TX pending, skipping");
        return;
    }

    float temperature = dht.readTemperature();
    float humidity    = dht.readHumidity();
    int   moisture    = analogRead(MOISTURE_PIN);

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("DHT read failed");
    } else {
        Serial.printf("Temp: %.1f C  Humidity: %.1f%%  Moisture: %d\n",
                      temperature, humidity, moisture);
    }

    uint8_t payload[6];
    int16_t  tempInt  = (int16_t)(temperature * 100);
    uint16_t humInt   = (uint16_t)(humidity * 100);
    uint16_t moistInt = (uint16_t)moisture;

    payload[0] = tempInt >> 8;
    payload[1] = tempInt & 0xFF;
    payload[2] = humInt >> 8;
    payload[3] = humInt & 0xFF;
    payload[4] = moistInt >> 8;
    payload[5] = moistInt & 0xFF;

    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    Serial.println("Packet queued");

    lastTxMillis = millis();
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
}

void onEvent(ev_t ev) {
    switch (ev) {
        case EV_JOINING:
            Serial.println("Joining...");
            joined = false;
            break;
        case EV_JOINED:
            Serial.println("Joined!");
            joined = true;
            LMIC_setAdrMode(1);
            break;
        case EV_TXCOMPLETE:
            Serial.println("TX complete");
            lastTxMillis = millis();
            break;
        case EV_JOIN_FAILED:
            Serial.println("Join failed");
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    dht.begin();

    // Init OLED (no RST pin — pin 16 is not safely driveable on this board)
    oled = new SSD1306Wire(0x3c, OLED_SDA, OLED_SCL);
    oled->init();
    oled->flipScreenVertically();
    drawDisplay("Starting...", -1);
    Serial.println("OLED init done");

    os_init();
    LMIC_reset();
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();

    // Calculate countdown
    int countdown = -1;
    if (lastTxMillis > 0) {
        unsigned long elapsed = (millis() - lastTxMillis) / 1000;
        countdown = (elapsed < TX_INTERVAL) ? (TX_INTERVAL - elapsed) : 0;
    }

    // Only redraw when countdown changes (avoids flicker)
    if (countdown != lastCountdown) {
        lastCountdown = countdown;
        const char* status = joined ? "Joined" : "Joining...";
        drawDisplay(status, countdown);
    }
}
