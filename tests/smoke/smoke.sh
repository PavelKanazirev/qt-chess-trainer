#!/usr/bin/env bash
set -euo pipefail
APP="${1:?usage: smoke.sh <path-to-trainer_app>}"
PORT="${SMOKE_PORT:-18080}"
ADMIN_PORT="${SMOKE_ADMIN_PORT:-18081}"
CFG_DIR="$(mktemp -d)"
CFG="$CFG_DIR/config.json"

# 1. The UI loads and the app exits cleanly
timeout 20 "$APP" --smoke --no-api --config "$CFG"

# 2. Both REST APIs answer while the app runs
"$APP" --config "$CFG" --api-port "$PORT" --admin-port "$ADMIN_PORT" &
PID=$!
trap 'kill "$PID" 2>/dev/null || true; rm -rf "$CFG_DIR"' EXIT

ready=0
for _ in $(seq 1 50); do
  if curl -fsS "http://127.0.0.1:$PORT/health" >/dev/null 2>&1; then ready=1; break; fi
  sleep 0.2
done
[ "$ready" -eq 1 ] || { echo "client API did not answer on port $PORT" >&2; exit 1; }
echo "health: $(curl -fsS "http://127.0.0.1:$PORT/health")"

code=$(curl -s -o /dev/null -w '%{http_code}' "http://127.0.0.1:$ADMIN_PORT/config")
[ "$code" = "401" ] || { echo "admin API without token returned $code, expected 401" >&2; exit 1; }

curl -fsS -X PUT -H 'X-Admin-Token: change-me' -H 'Content-Type: application/json' \
  -d '{"selector":"Random"}' "http://127.0.0.1:$ADMIN_PORT/config" >/dev/null
grep -q '"Random"' "$CFG" || { echo "config file was not updated" >&2; exit 1; }

echo "smoke OK"