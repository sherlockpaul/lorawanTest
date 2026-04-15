#!/usr/bin/env bash
# Run on your Windows machine (Git Bash / WSL) to build and push to the LXC.
# Usage: ./deploy.sh
set -e

LXC=root@192.168.0.149
LXC_PATH=/opt/lorawan

echo "==> Building frontend..."
cd frontend
npm run build
cd ..

echo "==> Deploying backend to $LXC:$LXC_PATH ..."
scp -r backend/app "$LXC:$LXC_PATH/"
scp backend/main.py backend/requirements.txt "$LXC:$LXC_PATH/"

echo "==> Deploying frontend build to $LXC:$LXC_PATH/static ..."
ssh "$LXC" "mkdir -p $LXC_PATH/static"
scp -r frontend/dist/. "$LXC:$LXC_PATH/static/"

echo ""
echo "==> Done. To apply changes, restart the backend on the LXC:"
echo "    ssh $LXC 'systemctl restart lorawan'"
echo ""
echo "==> Dashboard: http://192.168.0.149:8000"
