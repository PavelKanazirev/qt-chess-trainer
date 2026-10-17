#include "PuzzleSession.h"

PuzzleSession::PuzzleSession(Puzzle puzzle)
    : m_puzzle(std::move(puzzle)), m_pos(m_puzzle.fen) {
    // solution = opponent, player, opponent, player, ... so the count is even
    if (m_puzzle.moves.size() < 2 || m_puzzle.moves.size() % 2 != 0) return;
    m_valid = m_pos.play(m_puzzle.moves.first());
}

QString PuzzleSession::firstOpponentMove() const {
    return m_puzzle.moves.isEmpty() ? QString() : m_puzzle.moves.first();
}

MoveResult PuzzleSession::submit(const QString& uci) {
    MoveResult result;
    if (!m_valid || isFinished()) return result;   // verdict stays Illegal

    const QString expected = m_puzzle.moves.at(m_next);
    result.verdict = MoveValidator::check(m_pos, uci, expected);
    if (result.verdict == MoveVerdict::Illegal) return result;
    if (result.verdict == MoveVerdict::Wrong) {
        m_failed = true;
        return result;
    }

    m_pos.play(uci);
    ++m_next;
    if (m_next >= m_puzzle.moves.size() || m_pos.isCheckmate()) {
        m_solved = true;
        result.solved = true;
        return result;
    }
    playOpponentReplyIfAny(result);
    return result;
}

void PuzzleSession::playOpponentReplyIfAny(MoveResult& result) {
    const QString reply = m_puzzle.moves.at(m_next);
    m_pos.play(reply);
    result.opponentReply = reply;
    ++m_next;
    if (m_next >= m_puzzle.moves.size()) {
        m_solved = true;
        result.solved = true;
    }
}
