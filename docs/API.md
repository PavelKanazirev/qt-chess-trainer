# Client REST API

Started by the app on `http://127.0.0.1:8080` (change with `--api-port`, disable with
`--no-api`). Localhost only, no authentication. All bodies are JSON.

| Method | Path | Success | Errors |
|---|---|---|---|
| GET | `/health` | `200` `{status, puzzles, version}` | |
| POST | `/sessions` | `201` puzzle session | `503` no puzzle available |
| POST | `/sessions/{id}/moves` | `200` move outcome | `400` bad body, `404` unknown or finished session |
| GET | `/stats` | `200` player statistics | |

An illegal or wrong chess move is a valid request: the answer is `200` with
`"verdict": "illegal"` or `"wrong"`. A session closes when the puzzle is solved or failed.

## Example

    curl -s http://127.0.0.1:8080/health

    curl -s -X POST http://127.0.0.1:8080/sessions
    # {"fen":"...","lastMove":"e7e5","puzzleId":"0XYZ1","rating":1520,
    #  "sessionId":"6f1c...","sideToMove":"white","themes":["fork","short"]}

    curl -s -X POST http://127.0.0.1:8080/sessions/<sessionId>/moves \
         -H 'Content-Type: application/json' -d '{"move":"f3e5"}'
    # {"failed":false,"fen":"...","fenAfterMove":"...","opponentReply":"d8e7",
    #  "playerRating":1500,"solved":false,"verdict":"correct"}

    curl -s http://127.0.0.1:8080/stats | python3 -m json.tool

## Threading

cpp-httplib runs handlers on its own worker threads. They only call
`TrainerService`, whose public methods are serialized by one mutex.

# Admin REST API

Started on `http://127.0.0.1:8081` (`adminApiPort` in the config, or `--admin-port`).
Localhost only. Every request needs the header `X-Admin-Token`; without it, or with a
wrong token, the answer is `401`. `GET` shows the token as `***`.

| Method | Path | Body | Success | Errors |
|---|---|---|---|---|
| GET | `/config` | | `200` `{config, defaults, selectors, restartRequired}` | |
| PUT | `/config` | partial object, e.g. `{"selector":"Random"}` | `200` same as GET | `400` `{errors}` |
| POST | `/config/reset` | empty (all keys) or `{"keys":["selector"]}` | `200` same as GET | `400` `{errors}` |

| Key | Valid values | Applied |
|---|---|---|
| `selector` | `ByRating`, `Random`, `WeakestTheme` | immediately |
| `ratingWindow` | 50–1000 | immediately |
| `adminToken` | at least 8 characters | immediately |
| `clientApiPort`, `adminApiPort` | 1024–65535, different | after restart |
| `dataPath` | path, relative to the config file; empty = built-in sample | after restart |

    curl -s -H 'X-Admin-Token: change-me' http://127.0.0.1:8081/config
    curl -s -X PUT -H 'X-Admin-Token: change-me' -H 'Content-Type: application/json' \
         -d '{"selector":"WeakestTheme","ratingWindow":300}' http://127.0.0.1:8081/config
    curl -s -X POST -H 'X-Admin-Token: change-me' http://127.0.0.1:8081/config/reset
