#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

if [ ! -f .env ]; then
  cp .env.example .env
  echo "Created .env from .env.example. Edit backend/.env if you need custom settings."
fi

if [ ! -d node_modules ]; then
  echo "Installing Node.js dependencies..."
  npm install
fi

mkdir -p data
if [ ! -f data/licenses.json ]; then
  cat > data/licenses.json <<'EOF'
{
  "licenses": []
}
EOF
  echo "Created backend/data/licenses.json"
fi

if screen -list | grep -qE "\.[x]ash-license-server"; then
  echo "Screen session 'xash-license-server' is already running."
  exit 1
fi

echo "Starting xash3d license server in detached screen session 'xash-license-server'..."
screen -dmS xash-license-server npm start

echo "Server started. Attach with: screen -r xash-license-server"
