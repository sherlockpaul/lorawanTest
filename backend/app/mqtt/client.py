import os
import json
import base64
import struct
import paho.mqtt.client as mqtt
from dotenv import load_dotenv
from app.db.database import SessionLocal
from app.models.sensor_data import SensorReading

load_dotenv()

MQTT_HOST = os.getenv("MQTT_HOST", "localhost")
MQTT_PORT = int(os.getenv("MQTT_PORT", 1883))
APP_ID    = os.getenv("CHIRPSTACK_APPLICATION_ID", "1")

# ChirpStack v4 MQTT topic for uplink events
UPLINK_TOPIC = f"application/{APP_ID}/device/+/event/up"

_client: mqtt.Client | None = None


def _decode_payload(b64_payload: str) -> dict:
    """Decode the 6-byte sensor payload from the node firmware."""
    raw = base64.b64decode(b64_payload)
    if len(raw) < 6:
        return {}
    temp_raw, hum_raw, moist_raw = struct.unpack(">hHH", raw[:6])
    return {
        "temperature": temp_raw / 100.0,
        "humidity":    hum_raw / 100.0,
        "moisture":    moist_raw,
    }


def _on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload)
        device_eui  = data.get("deviceInfo", {}).get("devEui", "unknown")
        device_name = data.get("deviceInfo", {}).get("deviceName", None)
        b64_payload = data.get("data", "")
        readings    = _decode_payload(b64_payload)

        db = SessionLocal()
        reading = SensorReading(
            device_eui  = device_eui,
            device_name = device_name,
            **readings,
        )
        db.add(reading)
        db.commit()
        db.close()
        print(f"Stored reading from {device_eui}: {readings}")
    except Exception as e:
        print(f"MQTT message error: {e}")


def start_mqtt():
    global _client
    _client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    _client.on_message = _on_message
    _client.connect(MQTT_HOST, MQTT_PORT)
    _client.subscribe(UPLINK_TOPIC)
    _client.loop_start()
    print(f"MQTT connected to {MQTT_HOST}:{MQTT_PORT}, subscribed to {UPLINK_TOPIC}")


def stop_mqtt():
    if _client:
        _client.loop_stop()
        _client.disconnect()
