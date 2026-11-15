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
EOF

cat > scripts/check-doc-links.py <<'EOF'
#!/usr/bin/env python3
"""Fails if README.md or a file under docs/ links to a local file that does not exist."""
import re
import sys
from pathlib import Path

LINK = re.compile(r"\]\(([^)\s]+)\)")
root = Path(__file__).resolve().parents[1]

missing = []
for md in [root / "README.md", *sorted((root / "docs").rglob("*.md"))]:
    for target in LINK.findall(md.read_text(encoding="utf-8")):
        if re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", target) or target.startswith("#"):
            continue   # web links and in-page anchors
        if not (md.parent / target.split("#")[0]).exists():
            missing.append(f"{md.relative_to(root)}: {target}")

print("\n".join(missing) if missing else "all local links OK")
sys.exit(1 if missing else 0)
