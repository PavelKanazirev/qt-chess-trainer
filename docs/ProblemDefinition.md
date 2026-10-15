# Problem definition

Build a desktop chess tactics trainer. The user sees a position (from a FEN), finds the
best move and gets feedback. Puzzles come from a small CSV subset of the Lichess puzzle
database (CC0).

## Requirements

- R1: load puzzles from CSV (id, FEN, solution moves, rating, themes). 
Moves[0] is the opponent's move and is played automatically (see PuzzleFormat.md).
- R2: QML board shows the position and accepts click-click moves
- R3: a move is validated against the solution line; the opponent reply is auto-played
- R4: the puzzle selection algorithm is switchable at runtime
- R5: statistics: solved/failed counts and rating estimate per theme
- R6: client REST API: GET /health, GET /puzzles/next, POST /puzzles/{id}/move, GET /stats
- R7: admin REST API on 127.0.0.1:8081 with a token: GET/PUT /config, POST /config/reset
- R8: config.json holds selector name, rating window, ports, admin token and data path

## Out of scope


Engine analysis, online play, accounts.

## Acceptance

ctest passes, the app starts and shows a puzzle, and the admin GUI can change the selector
and the application reacts.
