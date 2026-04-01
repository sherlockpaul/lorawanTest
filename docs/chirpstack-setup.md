# ChirpStack v4 Setup on Proxmox LXC

## 1. Create the LXC Container in Proxmox

In the Proxmox web UI:

1. Click **Create CT**
2. Configure:
   - **Template**: Ubuntu 22.04 (download from Proxmox template list if needed)
   - **Hostname**: `chirpstack`
   - **Disk**: 8GB minimum
   - **CPU**: 2 cores
   - **RAM**: 1024 MB
   - **Network**: DHCP or assign a static IP (recommended — note this IP as `<lxc-ip>`)
   - **Unprivileged container**: Yes
3. Start the container and open its console

---

## 2. System Setup

```bash
apt update && apt upgrade -y
apt install -y curl wget gnupg apt-transport-https
```

---

## 3. Install PostgreSQL

```bash
apt install -y postgresql

# Start and enable
systemctl enable postgresql
systemctl start postgresql

# Create ChirpStack database and user
sudo -u postgres psql <<EOF
CREATE ROLE chirpstack WITH LOGIN PASSWORD 'chirpstack';
CREATE DATABASE chirpstack WITH OWNER chirpstack;
\c chirpstack
CREATE EXTENSION pg_trgm;
CREATE EXTENSION hstore;
EOF

# Create a separate DB for the app backend (used by the FastAPI server)
sudo -u postgres psql <<EOF
CREATE ROLE chirpstack_app WITH LOGIN PASSWORD 'changeme';
CREATE DATABASE lorawan_app WITH OWNER chirpstack_app;
EOF
```

---

## 4. Install Redis

```bash
apt install -y redis-server
systemctl enable redis-server
systemctl start redis-server
```

---

## 5. Install Mosquitto (MQTT Broker)

```bash
apt install -y mosquitto mosquitto-clients

# Allow anonymous connections (fine for local homelab testing)
cat > /etc/mosquitto/conf.d/chirpstack.conf <<EOF
listener 1883
allow_anonymous true
EOF

systemctl enable mosquitto
systemctl restart mosquitto
```

---

## 6. Install ChirpStack

```bash
# Add ChirpStack APT repository
mkdir -p /etc/apt/keyrings/
wget -q -O - https://artifacts.chirpstack.io/packages/chirpstack.key | gpg --dearmor > /etc/apt/keyrings/chirpstack.gpg

echo "deb [signed-by=/etc/apt/keyrings/chirpstack.gpg] https://artifacts.chirpstack.io/packages/4.x/deb stable main" | tee /etc/apt/sources.list.d/chirpstack.list

apt update
apt install -y chirpstack
```

---

## 7. Configure ChirpStack

Edit `/etc/chirpstack/chirpstack.toml`:

```bash
nano /etc/chirpstack/chirpstack.toml
```

Key sections to update:

```toml
[postgresql]
dsn = "postgres://chirpstack:chirpstack@localhost/chirpstack?sslmode=disable"

[redis]
servers = ["redis://localhost/"]

[network]
# Set your LoRaWAN region — use EU868, US915, AU915, etc.
enabled_regions = ["us915_0"]  # Change to match your region

[integration]
enabled = ["mqtt"]

[integration.mqtt]
server = "tcp://localhost:1883/"
```

> **Note:** US915 uses sub-band 0 (channels 0–7) by default. If using AU915 or EU868, change accordingly.

---

## 8. Start ChirpStack

```bash
systemctl enable chirpstack
systemctl start chirpstack

# Verify it's running
systemctl status chirpstack

# Watch logs
journalctl -u chirpstack -f
```

---

## 9. Access the Web UI

Open a browser on your home network and go to:

```
http://<lxc-ip>:8080
```

Default credentials:
- **Username**: `admin`
- **Password**: `admin`

> Change the password immediately after first login.

---

## 10. Initial ChirpStack Configuration

Once logged in:

1. **Create a Device Profile**
   - Go to **Device Profiles** → **Add device profile**
   - Name: `SensorNode`
   - Region: match your region (e.g. US915)
   - MAC version: LoRaWAN 1.0.3
   - Regional params: A
   - Enable **OTAA**

2. **Create an Application**
   - Go to **Applications** → **Add application**
   - Name: `LoRaWAN Test`
   - Note the **Application ID** — put it in your backend `.env` file

3. **Add a Device** (or use the simulator — see below)
   - Inside your application, click **Add device**
   - Fill in a name and DevEUI (can be random for testing)
   - After saving, set the **Application Key** under the OTAA tab
   - Copy DevEUI, AppEUI, and AppKey into `firmware/moisture_temp_node/moisture_temp_node.ino`

---

## 11. Test Without Hardware (Device Simulator)

ChirpStack includes a built-in device simulator:

1. Go to your application → select a device
2. Click the **Device simulator** tab (ChirpStack v4.9+)
   - Or use the `chirpstack-simulator` CLI tool for older versions

Alternatively, use **MQTT to inject fake uplink events** for development:

```bash
# Install mosquitto clients if not already
apt install -y mosquitto-clients

# Publish a fake uplink (replace APP_ID and DEV_EUI)
mosquitto_pub -h localhost -t "application/1/device/0102030405060708/event/up" \
  -m '{
    "deviceInfo": {
      "devEui": "0102030405060708",
      "deviceName": "test-node-1"
    },
    "data": "AAAAAAAAAA=="
  }'
```

This will trigger your FastAPI backend MQTT subscriber and insert a row into the database.

---

## Ports Reference

| Service | Port | Purpose |
|---|---|---|
| ChirpStack UI | 8080 | Web dashboard |
| MQTT broker | 1883 | Device uplink events |
| PostgreSQL | 5432 | Database |
| FastAPI backend | 8000 | REST API (run on your PC) |
| React frontend | 5173 | Dev server (Vite) |

---

## Troubleshooting

```bash
# ChirpStack logs
journalctl -u chirpstack -f

# Mosquitto logs
journalctl -u mosquitto -f

# Test MQTT subscription (watch for incoming messages)
mosquitto_sub -h localhost -t "application/#" -v

# PostgreSQL connection test
psql -U chirpstack -h localhost -d chirpstack
```
