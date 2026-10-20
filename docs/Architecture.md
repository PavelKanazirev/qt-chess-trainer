# Architecture

## Admin: change the selection strategy

Source: diagrams/admin-config-change.puml (CI renders it to SVG and uploads it as the
"diagrams" artifact of every workflow run).

The admin GUI sends PUT /config with a token. The admin server verifies the token,
ConfigService validates and persists the change, and TrainerService swaps.

## Chess logic boundary

`ChessPosition` is an adapter: it is the only code that includes `chess.hpp`
(chess-library, MIT, fetched with CMake FetchContent). Everything else works with
FEN strings and UCI move strings. `PuzzleSession` owns the per-puzzle state and
uses `MoveValidator`, which accepts the stored solution move or any legal move
that gives checkmate.

## Strategy: puzzle selection

`IPuzzleSelector` is the Strategy interface. Implementations:

| Name | Behaviour |
|---|---|
| Random | any puzzle not played recently |
| ByRating | random puzzle within playerRating ± ratingWindow, else the closest rating |
| WeakestTheme | the theme with most failures, closest rating; delegates to ByRating without data |

`SelectorRegistry` creates a selector from its name. The active name will come
from `config.json` , so the admin can switch strategies at runtime.
Selectors are pure: all input arrives through `SelectionContext`, and the
random generator is seeded, which keeps the tests deterministic.
