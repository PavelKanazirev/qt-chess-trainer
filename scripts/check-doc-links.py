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
