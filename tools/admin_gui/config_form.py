"""Editors for the trainer configuration. No networking: the window feeds it data."""
from __future__ import annotations

from PySide6.QtCore import Signal
from PySide6.QtWidgets import (QComboBox, QFormLayout, QHBoxLayout, QLineEdit, QSpinBox,
                               QToolButton, QWidget)

from admin_client import KEYS

LABELS = {
    "selector": "Strategy",
    "ratingWindow": "Rating window",
    "clientApiPort": "Client API port",
    "adminApiPort": "Admin API port",
    "adminToken": "Admin token",
    "dataPath": "Puzzle file",
}


def _spin(minimum: int, maximum: int, step: int = 1) -> QSpinBox:
    box = QSpinBox()
    box.setRange(minimum, maximum)
    box.setSingleStep(step)
    return box


class ConfigForm(QWidget):
    """One editor and one Default button per configuration key."""

    resetRequested = Signal(str)   # the key whose Default button was clicked

    def __init__(self, parent: QWidget | None = None):
        super().__init__(parent)
        self.editors = {
            "selector": QComboBox(),
            "ratingWindow": _spin(50, 1000, 50),
            "clientApiPort": _spin(1024, 65535),
            "adminApiPort": _spin(1024, 65535),
            "adminToken": QLineEdit(),
            "dataPath": QLineEdit(),
        }
        token = self.editors["adminToken"]
        token.setEchoMode(QLineEdit.EchoMode.Password)
        token.setPlaceholderText("unchanged (type a new token to change it)")
        self.editors["dataPath"].setPlaceholderText("built-in Lichess sample")

        self.default_buttons: dict[str, QToolButton] = {}
        self._loaded: dict = {}   # the values the server reported, per key

        layout = QFormLayout(self)
        for key in KEYS:
            button = QToolButton()
            button.setText("Default")
            button.clicked.connect(lambda _checked=False, k=key: self.resetRequested.emit(k))
            self.default_buttons[key] = button
            row = QHBoxLayout()
            row.addWidget(self.editors[key], 1)
            row.addWidget(button)
            layout.addRow(LABELS[key], row)

    def set_selectors(self, names: list[str]) -> None:
        box = self.editors["selector"]
        if [box.itemText(i) for i in range(box.count())] != list(names):
            box.clear()
            box.addItems(names)

    def show_config(self, config: dict, defaults: dict, keys=KEYS) -> None:
        """Show server values for 'keys'; editors of other keys keep their (unsaved) text."""
        for key in keys:
            self._loaded[key] = config[key]
            self._set_value(key, config[key])
        for key, button in self.default_buttons.items():
            if key == "adminToken":
                shown = "the default token"
            elif defaults[key] == "":
                shown = "the built-in sample"
            else:
                shown = str(defaults[key])
            button.setToolTip(f"Reset {LABELS[key].lower()} to {shown} now")

    def values(self) -> dict:
        return {key: self._value(key) for key in KEYS}

    def pending_changes(self) -> dict:
        """Keys whose editor differs from the server. An empty token field means unchanged."""
        changes = {}
        for key, value in self.values().items():
            if key == "adminToken":
                if value:
                    changes[key] = value
            elif key in self._loaded and value != self._loaded[key]:
                changes[key] = value
        return changes

    def _set_value(self, key: str, value) -> None:
        editor = self.editors[key]
        if isinstance(editor, QComboBox):
            editor.setCurrentText(str(value))
        elif isinstance(editor, QSpinBox):
            editor.setValue(int(value))
        elif key == "adminToken":
            editor.clear()   # the API only ever shows "***"
        else:
            editor.setText(str(value))

    def _value(self, key: str):
        editor = self.editors[key]
        if isinstance(editor, QComboBox):
            return editor.currentText()
        if isinstance(editor, QSpinBox):
            return editor.value()
        return editor.text().strip()
