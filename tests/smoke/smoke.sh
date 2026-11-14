#!/usr/bin/env bash
set -euo pipefail
APP="${1:?usage: smoke.sh <path-to-trainer_app>}"
PORT="${SMOKE_PORT:-18080}"

# 1. The UI loads and the app exits cleanly
timeout 20 "$APP" --smoke --no-api

# 2. The client REST API answers while the app runs
"$APP" --api-port "$PORT" &
PID=$!
trap 'kill "$PID" 2>/dev/null || true' EXIT

for _ in $(seq 1 50); do
  if body=$(curl -fsS "http://127.0.0.1:$PORT/health" 2>/dev/null); then
    echo "health: $body"
    echo "$body" | grep -q '"status":"ok"'
    echo "smoke OK"
    exit 0
  fi
  sleep 0.2
done

echo "API did not answer on port $PORT" >&2
exit 1

