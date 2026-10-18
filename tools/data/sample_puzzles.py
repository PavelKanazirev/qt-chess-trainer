#!/usr/bin/env python3
"""Pick a small, rating-balanced sample from the Lichess puzzle CSV (CC0).

Usage:
  curl -sL https://database.lichess.org/lichess_db_puzzle.csv.zst \
    | zstdcat | head -n 500000 \
    | python3 tools/data/sample_puzzles.py --per-bucket 40 > data/puzzles_lichess.csv
"""
import argparse
import sys

BUCKETS = [(0, 1000), (1000, 1400), (1400, 1800), (1800, 2200), (2200, 10000)]
HEADER = ("PuzzleId,FEN,Moves,Rating,RatingDeviation,Popularity,NbPlays,"
          "Themes,GameUrl,OpeningTags")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--per-bucket", type=int, default=40)
    ap.add_argument("--max-moves", type=int, default=6)
    ap.add_argument("--min-popularity", type=int, default=80)
    ap.add_argument("--min-plays", type=int, default=500)
    args = ap.parse_args()

    picked = [[] for _ in BUCKETS]
    for raw in sys.stdin:
        line = raw.rstrip("\r\n")
        f = line.split(",")
        if len(f) < 8 or f[0] == "PuzzleId":
            continue
        try:
            rating, popularity, plays = int(f[3]), int(f[5]), int(f[6])
        except ValueError:
            continue
        moves = f[2].split()
        if len(moves) < 2 or len(moves) % 2 or len(moves) > args.max_moves:
            continue
        if popularity < args.min_popularity or plays < args.min_plays:
            continue
        for i, (lo, hi) in enumerate(BUCKETS):
            if lo <= rating < hi and len(picked[i]) < args.per_bucket:
                picked[i].append(line)
                break
        if all(len(b) >= args.per_bucket for b in picked):
            break

    print(HEADER)
    for bucket in picked:
        for line in bucket:
            print(line)
    for (lo, hi), bucket in zip(BUCKETS, picked):
        print(f"rating {lo}-{hi}: {len(bucket)} puzzles", file=sys.stderr)


if __name__ == "__main__":
    main()
