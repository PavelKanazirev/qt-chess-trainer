#!/usr/bin/env bash
set -euo pipefail
APP="${1:?usage: smoke.sh <path-to-trainer_app>}"
timeout 20 "$APP" --smoke
echo "smoke OK"
