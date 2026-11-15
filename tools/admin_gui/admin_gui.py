#!/usr/bin/env python3
"""PySide6 admin GUI for the Qt Chess Trainer: view, edit and reset its configuration."""
from __future__ import annotations

import argparse
import os
import sys

from PySide6.QtCore import QTimer
from PySide6.QtWidgets import (QApplication, QHBoxLayout, QLabel, QLineEdit, QMainWindow,
                               QMessageBox, QPushButton, QVBoxLayout, QWidget)

from admin_client import KEYS, AdminClient, AdminError, ConfigView
from config_form import ConfigForm

# TrainerConfig's default token. The API masks tokens, so after a reset the GUI must know it.
DEFAULT_TOKEN = "change-me"


def _restart_note(view: ConfigView) -> str:
    if not view.restart_required:
        return ""
    return " Restart the trainer to apply: " + ", ".join(view.restart_required) + "."


class AdminWindow(QMainWindow):
    def __init__(self, url: str, token: str, client_factory=AdminClient):
        super().__init__()
        self.setWindowTitle("Chess Trainer Admin")
        self._factory = client_factory
        self.client = None

        self.url_edit = QLineEdit(url)
        self.token_edit = QLineEdit(token)
        self.token_edit.setEchoMode(QLineEdit.EchoMode.Password)
        self.connect_button = QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_to_server)

        self.form = ConfigForm()
        self.form.resetRequested.connect(lambda key: self.reset_keys([key]))

        self.message = QLabel()
        self.message.setWordWrap(True)

        self.reload_button = QPushButton("Reload")
        self.reload_button.clicked.connect(self.reload)
        self.save_button = QPushButton("Save")
        self.save_button.clicked.connect(self.save)
        self.reset_all_button = QPushButton("Reset all to defaults")
        self.reset_all_button.clicked.connect(self.reset_all)

        connection = QHBoxLayout()
        connection.addWidget(QLabel("URL"))
        connection.addWidget(self.url_edit, 2)
        connection.addWidget(QLabel("Token"))
        connection.addWidget(self.token_edit, 1)
        connection.addWidget(self.connect_button)

        buttons = QHBoxLayout()
        buttons.addWidget(self.reload_button)
        buttons.addStretch()
        buttons.addWidget(self.save_button)
        buttons.addWidget(self.reset_all_button)

        layout = QVBoxLayout()
        layout.addLayout(connection)
        layout.addWidget(self.form)
        layout.addWidget(self.message)
        layout.addLayout(buttons)
        central = QWidget()
        central.setLayout(layout)
        self.setCentralWidget(central)
        self._set_editable(False)

    # --- actions -------------------------------------------------------------------------

    def connect_to_server(self) -> None:
        self.client = self._factory(self.url_edit.text().strip(), self.token_edit.text())
        self.reload()

    def reload(self) -> None:
        view = self._call(self.client.get_config)
        if view:
            self._show(view)
            self._info("Loaded the current configuration." + _restart_note(view))

    def save(self) -> None:
        patch = self.form.pending_changes()
        if not patch:
            self._info("Nothing to save.")
            return
        view = self._call(self.client.put_config, patch)
        if view is None:
            return
        if "adminToken" in patch:
            self._use_token(patch["adminToken"])
        self._show(view)
        self._info("Saved: " + ", ".join(sorted(patch)) + "." + _restart_note(view))

    def reset_keys(self, keys: list[str]) -> None:
        view = self._call(self.client.reset, keys)
        if view is None:
            return
        if "adminToken" in keys:
            self._use_token(DEFAULT_TOKEN)
        self._show(view, keys)   # other editors keep their unsaved edits
        self._info("Reset to default: " + ", ".join(keys) + "." + _restart_note(view))

    def reset_all(self) -> None:
        if not self.confirm("Reset every setting, including the admin token, to its default?"):
            return
        view = self._call(self.client.reset)
        if view is None:
            return
        self._use_token(DEFAULT_TOKEN)
        self._show(view)
        self._info("All settings reset to their defaults." + _restart_note(view))

    def confirm(self, text: str) -> bool:   # replaced in tests: no modal dialogs there
        answer = QMessageBox.question(self, "Confirm", text)
        return answer == QMessageBox.StandardButton.Yes

    # --- helpers -------------------------------------------------------------------------

    def _call(self, function, *args):
        try:
            return function(*args)
        except AdminError as error:
            self._error(str(error))
            if error.status in (0, 401):   # unreachable or wrong token: reconnect first
                self._set_editable(False)
            return None

    def _show(self, view: ConfigView, keys=KEYS) -> None:
        self.form.set_selectors(view.selectors)
        self.form.show_config(view.config, view.defaults, keys)
        self._set_editable(True)

    def _use_token(self, token: str) -> None:
        self.client.token = token
        self.token_edit.setText(token)

    def _set_editable(self, editable: bool) -> None:
        self.form.setEnabled(editable)
        for button in (self.reload_button, self.save_button, self.reset_all_button):
            button.setEnabled(editable)

    def _info(self, text: str) -> None:
        self.message.setStyleSheet("color: #2e7d32;")
        self.message.setText(text)

    def _error(self, text: str) -> None:
        self.message.setStyleSheet("color: #c62828;")
        self.message.setText(text)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default=os.environ.get("TRAINER_ADMIN_URL",
                                                        "http://127.0.0.1:8081"))
    parser.add_argument("--token", default=os.environ.get("TRAINER_ADMIN_TOKEN", DEFAULT_TOKEN),
                        help="prefer the TRAINER_ADMIN_TOKEN environment variable")
    args = parser.parse_args(argv)

    app = QApplication(sys.argv[:1])
    window = AdminWindow(args.url, args.token)
    window.resize(640, 420)
    window.show()
    QTimer.singleShot(0, window.connect_to_server)
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
EOF

cat > tools/admin_gui/tests/test_admin_gui.py <<'EOF'
import pytest
from conftest import DEFAULTS, SELECTORS

from admin_client import AdminError, ConfigView

RESTART_KEYS = ("clientApiPort", "adminApiPort", "dataPath")


class FakeClient:
    """Behaves like AdminClient against a running trainer, without any network."""

    def __init__(self, url, token):
        self.url = url
        self.token = token
        self.config = dict(DEFAULTS)
        self.calls = []
        self.fail_next = None   # messages for the next put_config to reject with 400

    def _view(self):
        restart = [k for k in RESTART_KEYS if self.config[k] != DEFAULTS[k]]
        return ConfigView({**self.config, "adminToken": "***"},
                          {**DEFAULTS, "adminToken": "***"}, SELECTORS, restart)

    def _check(self):
        if self.token != self.config["adminToken"]:
            raise AdminError(401, ["missing or wrong X-Admin-Token"])

    def get_config(self):
        self._check()
        self.calls.append(("get",))
        return self._view()

    def put_config(self, patch):
        self._check()
        self.calls.append(("put", patch))
        if self.fail_next:
            messages, self.fail_next = self.fail_next, None
            raise AdminError(400, messages)
        self.config.update(patch)
        return self._view()

    def reset(self, keys=None):
        self._check()
        self.calls.append(("reset", keys))
        for key in keys or DEFAULTS:
            self.config[key] = DEFAULTS[key]
        return self._view()


def make_window(token="change-me"):
    from admin_gui import AdminWindow
    created = []

    def factory(url, tok):
        created.append(FakeClient(url, tok))
        return created[-1]

    window = AdminWindow("http://test", token, client_factory=factory)
    window.connect_to_server()
    window.fake = created[-1]
    return window


@pytest.fixture
def window(qapp):
    w = make_window()
    yield w
    w.close()


def test_connect_shows_current_config(window):
    assert window.form.editors["selector"].currentText() == "ByRating"
    assert window.form.editors["selector"].count() == 3
    assert window.save_button.isEnabled()
    assert "Loaded" in window.message.text()


def test_wrong_token_disables_editing(qapp):
    w = make_window(token="nope")
    assert not w.save_button.isEnabled()
    assert "X-Admin-Token" in w.message.text()
    w.close()


def test_save_sends_only_changes(window):
    window.form.editors["ratingWindow"].setValue(300)
    window.save()
    assert window.fake.calls[-1] == ("put", {"ratingWindow": 300})
    assert "Saved" in window.message.text()


def test_nothing_to_save(window):
    calls = len(window.fake.calls)
    window.save()
    assert len(window.fake.calls) == calls
    assert "Nothing" in window.message.text()


def test_server_error_keeps_the_edit(window):
    window.fake.fail_next = ["dataPath: no such file"]
    window.form.editors["dataPath"].setText("missing.csv")
    window.save()
    assert "no such file" in window.message.text()
    assert window.form.editors["dataPath"].text() == "missing.csv"
    assert window.save_button.isEnabled()   # a 400 does not disconnect


def test_port_change_reports_restart(window):
    window.form.editors["clientApiPort"].setValue(9090)
    window.save()
    assert "Restart" in window.message.text()
    assert "clientApiPort" in window.message.text()


def test_token_change_keeps_the_connection(window):
    window.form.editors["adminToken"].setText("new-token-1")
    window.save()
    assert window.client.token == "new-token-1"
    assert window.form.editors["adminToken"].text() == ""
    window.reload()                                   # still authorized
    assert window.fake.calls[-1] == ("get",)


def test_default_button_keeps_other_edits(window):
    window.fake.config.update(selector="Random", ratingWindow=300)
    window.reload()
    window.form.editors["ratingWindow"].setValue(400)   # unsaved
    window.form.default_buttons["selector"].click()
    assert window.fake.calls[-1] == ("reset", ["selector"])
    assert window.form.editors["selector"].currentText() == "ByRating"
    assert window.form.editors["ratingWindow"].value() == 400
    assert window.form.pending_changes() == {"ratingWindow": 400}


def test_reset_all_asks_first(window):
    window.fake.config.update(selector="Random", ratingWindow=300)
    window.reload()

    window.confirm = lambda text: False
    window.reset_all_button.click()
    assert window.fake.calls[-1] == ("get",)            # nothing reset

    window.confirm = lambda text: True
    window.reset_all_button.click()
    assert window.fake.calls[-1] == ("reset", None)
    assert window.form.editors["selector"].currentText() == "ByRating"
    assert window.form.editors["ratingWindow"].value() == 200
