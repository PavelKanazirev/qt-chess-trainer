"""HTTP client for the Qt Chess Trainer admin REST API (standard library only)."""
from __future__ import annotations

import json
import urllib.error
import urllib.request
from dataclasses import dataclass, field

KEYS = ("selector", "ratingWindow", "clientApiPort", "adminApiPort", "adminToken", "dataPath")


class AdminError(Exception):
    """A failed admin request. status is the HTTP status, or 0 if the server was unreachable."""

    def __init__(self, status: int, messages: list[str]):
        super().__init__("; ".join(messages) or f"HTTP {status}")
        self.status = status
        self.messages = messages


@dataclass
class ConfigView:
    config: dict
    defaults: dict
    selectors: list[str]
    restart_required: list[str] = field(default_factory=list)

    @classmethod
    def from_json(cls, body: dict) -> "ConfigView":
        return cls(
            config=dict(body["config"]),
            defaults=dict(body["defaults"]),
            selectors=list(body["selectors"]),
            restart_required=list(body.get("restartRequired", [])),
        )


def _messages(raw: bytes) -> list[str]:
    try:
        body = json.loads(raw)
    except (ValueError, UnicodeDecodeError):
        text = raw.decode(errors="replace").strip()
        return [text] if text else []
    if isinstance(body, dict):
        if isinstance(body.get("errors"), list):
            return [str(m) for m in body["errors"]]
        if "error" in body:
            return [str(body["error"])]
    return [str(body)]


class AdminClient:
    def __init__(self, base_url: str = "http://127.0.0.1:8081", token: str = "",
                 timeout: float = 3.0):
        self.base_url = base_url.rstrip("/")
        self.token = token
        self.timeout = timeout
        # The admin API is on localhost: never route it through an HTTP proxy.
        self._opener = urllib.request.build_opener(urllib.request.ProxyHandler({}))

    def get_config(self) -> ConfigView:
        return ConfigView.from_json(self._request("GET", "/config"))

    def put_config(self, patch: dict) -> ConfigView:
        return ConfigView.from_json(self._request("PUT", "/config", patch))

    def reset(self, keys: list[str] | None = None) -> ConfigView:
        """Reset the given keys to their defaults; None resets every key."""
        body = {"keys": list(keys)} if keys else None
        return ConfigView.from_json(self._request("POST", "/config/reset", body))

    def _request(self, method: str, path: str, body: dict | None = None) -> dict:
        data = None if body is None else json.dumps(body).encode()
        request = urllib.request.Request(self.base_url + path, data=data, method=method)
        request.add_header("X-Admin-Token", self.token)
        if data is not None:
            request.add_header("Content-Type", "application/json")
        try:
            with self._opener.open(request, timeout=self.timeout) as response:
                return json.loads(response.read() or b"{}")
        except urllib.error.HTTPError as error:
            raise AdminError(error.code, _messages(error.read())) from None
        except (urllib.error.URLError, OSError) as error:
            reason = getattr(error, "reason", error)
            raise AdminError(0, [f"cannot reach {self.base_url}: {reason}"]) from None
