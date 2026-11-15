import os
import sys
from pathlib import Path

import pytest

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))   # tools/admin_gui

DEFAULTS = {
    "selector": "ByRating",
    "ratingWindow": 200,
    "clientApiPort": 8080,
    "adminApiPort": 8081,
    "adminToken": "change-me",
    "dataPath": "",
}
SELECTORS = ["ByRating", "Random", "WeakestTheme"]


@pytest.fixture(scope="session")
def qapp():
    from PySide6.QtWidgets import QApplication
    app = QApplication.instance() or QApplication([])
    yield app
