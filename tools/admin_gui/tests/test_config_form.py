import pytest
from conftest import DEFAULTS, SELECTORS


@pytest.fixture
def form(qapp):
    from config_form import ConfigForm
    f = ConfigForm()
    f.set_selectors(SELECTORS)
    f.show_config({**DEFAULTS, "adminToken": "***", "selector": "Random"},
                  {**DEFAULTS, "adminToken": "***"})
    yield f
    f.deleteLater()


def test_shows_server_values(form):
    assert form.editors["selector"].currentText() == "Random"
    assert form.editors["ratingWindow"].value() == 200
    assert form.editors["adminToken"].text() == ""      # never shows "***"
    assert form.pending_changes() == {}


def test_reports_only_changed_keys(form):
    form.editors["ratingWindow"].setValue(300)
    form.editors["selector"].setCurrentText("WeakestTheme")
    assert form.pending_changes() == {"ratingWindow": 300, "selector": "WeakestTheme"}


def test_empty_token_means_unchanged(form):
    assert "adminToken" not in form.pending_changes()
    form.editors["adminToken"].setText("new-token-1")
    assert form.pending_changes() == {"adminToken": "new-token-1"}


def test_partial_update_keeps_other_edits(form):
    form.editors["ratingWindow"].setValue(400)                      # unsaved edit
    form.show_config({**DEFAULTS, "adminToken": "***"}, DEFAULTS, keys=["selector"])
    assert form.editors["selector"].currentText() == "ByRating"
    assert form.editors["ratingWindow"].value() == 400
    assert form.pending_changes() == {"ratingWindow": 400}


def test_default_button_emits_its_key(form):
    seen = []
    form.resetRequested.connect(seen.append)
    form.default_buttons["dataPath"].click()
    assert seen == ["dataPath"]
