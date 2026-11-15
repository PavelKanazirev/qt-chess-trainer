#!/usr/bin/env bash
# Installs the Ubuntu packages needed to build and test the project, or to render its docs.
# Used by CI and by the README. Usage: scripts/install-deps.sh [--build] [--docs]
set -euo pipefail

BUILD_PACKAGES=(
  build-essential cmake ninja-build git curl
  qt6-base-dev qt6-declarative-dev libgl1-mesa-dev
  qml6-module-qtqml qml6-module-qtqml-workerscript qml6-module-qtqml-models
  qml6-module-qtquick qml6-module-qtquick-window qml6-module-qtquick-templates
  qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qttest
)
DOCS_PACKAGES=(plantuml graphviz)

if [ $# -eq 0 ]; then
  set -- --build
fi

packages=()
for arg in "$@"; do
  case "$arg" in
    --build) packages+=("${BUILD_PACKAGES[@]}") ;;
    --docs)  packages+=("${DOCS_PACKAGES[@]}") ;;
    *) echo "usage: $0 [--build] [--docs]" >&2; exit 2 ;;
  esac
done

SUDO=""
if [ "$(id -u)" -ne 0 ]; then
  SUDO="sudo"
fi
$SUDO apt-get update
$SUDO apt-get install -y "${packages[@]}"
