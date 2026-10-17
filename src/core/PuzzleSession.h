#pragma once
#include <QString>
#include "ChessPosition.h"
#include "MoveValidator.h"
#include "Puzzle.h"

struct MoveResult {
    MoveVerdict verdict = MoveVerdict::Illegal;
    QString opponentReply;   // auto-played reply, empty if none
    bool solved = false;
};

// State machine for one puzzle. The opponent's first move is played on construction.
class PuzzleSession {
public:
    explicit PuzzleSession(Puzzle puzzle);

    bool isValid() const { return m_valid; }
    bool isSolved() const { return m_solved; }
    bool isFailed() const { return m_failed; }
    bool isFinished() const { return m_solved || m_failed; }

    const Puzzle& puzzle() const { return m_puzzle; }
    QString fen() const { return m_pos.fen(); }
    QString firstOpponentMove() const;

    MoveResult submit(const QString& uci);

private:
    void playOpponentReplyIfAny(MoveResult& result);

    Puzzle m_puzzle;
    ChessPosition m_pos;
    int m_next = 1;          // index into puzzle.moves of the next expected player move
    bool m_valid = false;
    bool m_solved = false;
    bool m_failed = false;
};
