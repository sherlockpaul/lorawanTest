// LoRaWAN Moisture + Temperature Node
// Target hardware: TTGO LoRa32 (ESP32 + SX1276)
// Framework: Arduino
//
// Required libraries (install via Arduino Library Manager):
//   - MCCI LoRaWAN LMIC library
//   - DHT sensor library (Adafruit)
//   - Adafruit Unified Sensor

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <DHT.h>

// ---------- Sensor config ----------
#define DHT_PIN     13
#define DHT_TYPE    DHT22
#define MOISTURE_PIN 34   // ADC pin for capacitive moisture sensor

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

// ---------- Transmit interval ----------
static osjob_t sendjob;
const unsigned TX_INTERVAL = 60; // seconds

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

    // Encode payload: 2 bytes temp, 2 bytes humidity, 2 bytes moisture
    uint8_t payload[6];
    int16_t tempInt     = (int16_t)(temperature * 100);
    uint16_t humInt     = (uint16_t)(humidity * 100);
    uint16_t moistInt   = (uint16_t)moisture;

    payload[0] = tempInt >> 8;
    payload[1] = tempInt & 0xFF;
    payload[2] = humInt >> 8;
    payload[3] = humInt & 0xFF;
    payload[4] = moistInt >> 8;
    payload[5] = moistInt & 0xFF;

    LMIC_setTxData2(1, payload, sizeof(payload), 0);
    Serial.println("Packet queued");

    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
}

void onEvent(ev_t ev) {
    switch (ev) {
        case EV_JOINING:    Serial.println("Joining..."); break;
        case EV_JOINED:     Serial.println("Joined!"); LMIC_setAdrMode(1); break;
        case EV_TXCOMPLETE: Serial.println("TX complete"); break;
        case EV_JOIN_FAILED: Serial.println("Join failed"); break;
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    dht.begin();

    os_init();
    LMIC_reset();
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
