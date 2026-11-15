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
