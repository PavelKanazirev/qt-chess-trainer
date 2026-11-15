#!/usr/bin/env python3
"""End-to-end check: start trainer_app and drive its admin API through AdminClient."""
import argparse
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from admin_client import AdminClient, AdminError  # noqa: E402


def expect_error(status, function, *args):
    try:
        function(*args)
    except AdminError as error:
        assert error.status == status, f"expected HTTP {status}, got {error.status}: {error}"
        return
    raise AssertionError(f"expected HTTP {status}, the request succeeded")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("app")
    parser.add_argument("--api-port", type=int, default=18090)
    parser.add_argument("--admin-port", type=int, default=18091)
    args = parser.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        config = Path(tmp) / "config.json"
        env = {**os.environ, "QT_QPA_PLATFORM": os.environ.get("QT_QPA_PLATFORM", "offscreen")}
        app = subprocess.Popen([args.app, "--config", str(config),
                                "--api-port", str(args.api_port),
                                "--admin-port", str(args.admin_port)], env=env)
        try:
            client = AdminClient(f"http://127.0.0.1:{args.admin_port}", "change-me")
            view = None
            for _ in range(50):
                try:
                    view = client.get_config()
                    break
                except AdminError:
                    time.sleep(0.2)
            assert view is not None, "admin API did not answer"
            assert view.config["adminToken"] == "***"

            expect_error(401, AdminClient(client.base_url, "wrong-token-0").get_config)

            view = client.put_config({"selector": "WeakestTheme", "ratingWindow": 300})
            assert view.config["selector"] == "WeakestTheme"
            expect_error(400, client.put_config, {"ratingWindow": 5})

            view = client.reset(["ratingWindow"])
            assert view.config["ratingWindow"] == 200
            assert view.config["selector"] == "WeakestTheme"
            assert '"WeakestTheme"' in config.read_text()
            print("admin e2e OK")
            return 0
        finally:
            app.terminate()
            try:
                app.wait(timeout=5)
            except subprocess.TimeoutExpired:
                app.kill()


if __name__ == "__main__":
    sys.exit(main())
