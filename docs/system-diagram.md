# LoRaWAN System Diagram

```mermaid
graph TD

    subgraph NODE["Sensor Node (×N) — TTGO LoRa32"]
        DHT22["DHT22\nTemp/Humidity"]
        MOIST["Capacitive\nMoisture Sensor"]
        RELAY["Relay Module\n(Control Node)"]
        LIPO["LiPo Battery\n3.7V"]
        ESP32["ESP32\nFirmware (C++/Arduino)\n- LoRaWAN stack (OTAA)\n- Deep sleep scheduler"]
        ANT_NODE["LoRa Antenna"]

        DHT22 -->|GPIO| ESP32
        MOIST -->|ADC| ESP32
        RELAY -->|GPIO| ESP32
        LIPO -->|Power| ESP32
        ESP32 --> ANT_NODE
    end

    subgraph GW["LoRaWAN Gateway — Raspberry Pi 5"]
        ANT_GW["LoRa Antenna"]
        RAK2287["RAK2287\nConcentrator\n(SX1302)"]
        PIHAT["RAK2287\nPi HAT"]
        RPI["Raspberry Pi 5\nChirpStack\nPacket Forwarder"]

        ANT_GW --> RAK2287
        RAK2287 --> PIHAT
        PIHAT -->|SPI| RPI
    end

    subgraph LXC["Proxmox LXC — Network Server"]
        CHIRP["ChirpStack\nLoRaWAN Network Server"]
        MQTT["MQTT Broker\n(Mosquitto)"]

        CHIRP -->|Uplink events| MQTT
        MQTT -->|Downlink commands| CHIRP
    end

    subgraph BACK["Backend — Python FastAPI"]
        SUB["MQTT Subscriber"]
        DB["PostgreSQL"]
        API["REST API\n+ WebSockets"]

        SUB -->|Store readings| DB
        DB --> API
        API -->|Publish downlink| MQTT
    end

    subgraph FRONT["Frontend — React"]
        DASH["Live Dashboard\n(sensor readings)"]
        STATUS["Node Status\n& History"]
        CTRL["Control Interface\n(relay on/off)"]
    end

    ANT_NODE  -.->|"LoRaWAN RF\n(868/915 MHz)"| ANT_GW
    RPI       -->|"UDP / Semtech\nprotocol"| CHIRP
    MQTT      --> SUB
    API       <-->|WebSocket| DASH
    API       <-->|WebSocket| STATUS
    API       <-->|REST + WS| CTRL
```

## Protocol Stack

| Layer | Technology |
|---|---|
| RF | LoRa (spread spectrum) |
| MAC | LoRaWAN Class A (OTAA) |
| Transport (GW→NS) | UDP Semtech packet forwarder |
| Network Server | ChirpStack |
| App messaging | MQTT |
| API | HTTP REST + WebSockets |
| Frontend | React |

## Data Flow Summary

1. **Uplink**: Sensor reads → ESP32 encodes payload → LoRa TX → Gateway receives → Packet Forwarder → ChirpStack decodes → MQTT publish → FastAPI stores in PostgreSQL → WebSocket push → React dashboard
2. **Downlink**: User clicks control in React → REST call to FastAPI → MQTT publish → ChirpStack queues downlink → Gateway transmits → ESP32 toggles relay
