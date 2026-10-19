# Puzzle file format

CSV, one puzzle per line, optional header row. The columns match the Lichess puzzle
database (CC0) so real data can be dropped in later.

| Column | Meaning |
|---|---|
| PuzzleId | unique id |
| FEN | position BEFORE the opponent's first move |
| Moves | space-separated UCI moves; Moves[0] is the opponent's move, auto-played |
| Rating | puzzle difficulty, positive integer |
| Themes | space-separated theme tags |

The other columns are read but ignored.

`data/puzzles_sample.csv` contains three hand-made mate-in-one puzzles used by the
tests. It is not part of the Lichess data.

## Data sources

- `data/puzzles_sample.csv`: three hand-made puzzles for unit tests.
- `data/puzzles_lichess.csv`: 200 puzzles (40 per rating band) from the Lichess
  puzzle database (https://database.lichess.org/#puzzles), released under CC0.
  Regenerate with `tools/data/sample_puzzles.py` (usage in the script header).