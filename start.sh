#!/usr/bin/env bash
# Run on the LXC to start the backend.
# For auto-start on boot, install the systemd service instead (see lorawan.service).
set -e
cd "$(dirname "$0")"
source venv/bin/activate
exec uvicorn main:app --host 0.0.0.0 --port 8000
