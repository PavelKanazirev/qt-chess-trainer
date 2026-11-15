import json
import socket
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import pytest
from conftest import DEFAULTS, SELECTORS

from admin_client import AdminClient, AdminError

TOKEN = "secret-token"


class FakeAdminHandler(BaseHTTPRequestHandler):
    """A tiny stand-in for the C++ admin API."""

    def log_message(self, *args):
        pass

    def _send(self, status, body):
        data = json.dumps(body).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def _authorized(self):
        if self.headers.get("X-Admin-Token") != TOKEN:
            self._send(401, {"error": "missing or wrong X-Admin-Token"})
            return False
        return True

    def _body(self):
        length = int(self.headers.get("Content-Length", 0))
        return json.loads(self.rfile.read(length)) if length else None

    def _view(self):
        return {
            "config": {**self.server.config, "adminToken": "***"},
            "defaults": {**DEFAULTS, "adminToken": "***"},
            "selectors": SELECTORS,
            "restartRequired": [],
        }

    def do_GET(self):
        if self._authorized():
            self._send(200, self._view())

    def do_PUT(self):
        if not self._authorized():
            return
        patch = self._body() or {}
        unknown = [k for k in patch if k not in DEFAULTS]
        if unknown:
            self._send(400, {"errors": [f"unknown key: {k}" for k in unknown]})
            return
        self.server.config.update(patch)
        self._send(200, self._view())

    def do_POST(self):
        if not self._authorized():
            return
        body = self._body()
        keys = (body or {}).get("keys") or list(DEFAULTS)
        self.server.last_reset = (body, keys)
        for key in keys:
            self.server.config[key] = DEFAULTS[key]
        self._send(200, self._view())


@pytest.fixture
def server():
    srv = ThreadingHTTPServer(("127.0.0.1", 0), FakeAdminHandler)
    srv.config = dict(DEFAULTS)
    srv.last_reset = None
    thread = threading.Thread(target=srv.serve_forever, daemon=True)
    thread.start()
    yield srv
    srv.shutdown()
    srv.server_close()


def client_for(srv, token=TOKEN):
    return AdminClient(f"http://127.0.0.1:{srv.server_address[1]}", token)


def test_get_config_parses_the_view(server):
    view = client_for(server).get_config()
    assert view.config["selector"] == "ByRating"
    assert view.config["adminToken"] == "***"
    assert view.selectors == SELECTORS


def test_wrong_token_raises_401(server):
    with pytest.raises(AdminError) as error:
        client_for(server, "wrong").get_config()
    assert error.value.status == 401
    assert "X-Admin-Token" in str(error.value)


def test_put_sends_patch_and_reports_errors(server):
    view = client_for(server).put_config({"selector": "Random"})
    assert view.config["selector"] == "Random"

    with pytest.raises(AdminError) as error:
        client_for(server).put_config({"colour": "blue"})
    assert error.value.status == 400
    assert error.value.messages == ["unknown key: colour"]


def test_reset_one_key_or_all(server):
    client = client_for(server)
    client.put_config({"selector": "Random", "ratingWindow": 300})

    view = client.reset(["selector"])
    assert server.last_reset[0] == {"keys": ["selector"]}
    assert view.config["selector"] == "ByRating"
    assert view.config["ratingWindow"] == 300

    view = client.reset()
    assert server.last_reset[0] is None          # no body: reset everything
    assert view.config["ratingWindow"] == 200


def test_unreachable_server_raises_status_0():
    with socket.socket() as s:                   # a port with nothing listening
        s.bind(("127.0.0.1", 0))
        port = s.getsockname()[1]
    with pytest.raises(AdminError) as error:
        AdminClient(f"http://127.0.0.1:{port}", TOKEN, timeout=1).get_config()
    assert error.value.status == 0
