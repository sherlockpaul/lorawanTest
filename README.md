# LoRaWAN Sensor Network

A LoRaWAN-based IoT sensor network for monitoring moisture, temperature, and control nodes via a web dashboard.

## Architecture

```
[Sensor Nodes]         [RPi 5 + LoRa HAT]     [Proxmox LXC]
  C++ / Arduino   →    Packet Forwarder   →   ChirpStack (LoRaWAN server)
  - Moisture                                          ↓ MQTT
  - Temperature                            [Python FastAPI backend]
  - Control relay                            - MQTT subscriber
                                             - PostgreSQL storage
                                             - REST API / WebSockets
                                                      ↓
                                            [React frontend]
                                             - Live dashboard
                                             - Node status & history
                                             - Control interface
```

## Project Structure

```
lorawanTest/
├── firmware/           # C++ Arduino sketches for sensor nodes
├── backend/            # Python FastAPI server
├── frontend/           # React web UI
└── docs/               # Setup guides and documentation
```

## Setup Guides

- [ChirpStack on Proxmox LXC](docs/chirpstack-setup.md)

## Hardware (Phase 2)

| Component | Model | Purpose |
|---|---|---|
| Gateway radio | RAK2287 HAT | LoRa concentrator for RPi 5 |
| Sensor nodes | TTGO LoRa32 (ESP32) | LoRa + WiFi dev board |
| Temp sensor | DHT22 or DS18B20 | Temperature / humidity |
| Moisture sensor | Capacitive soil sensor | Soil moisture |
