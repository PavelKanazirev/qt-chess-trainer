#include "TrainerBridge.h"

#include <QTimer>

TrainerBridge::TrainerBridge(TrainerService& service, QObject* parent)
    : QObject(parent), m_service(service), m_board(new BoardModel(this)) {
    refreshStats();
}

void TrainerBridge::nextPuzzle() {
    ++m_generation;
    m_finished = false;
    m_waitingForReply = false;

    const auto view = m_service.startNext();
    if (!view) {
        m_finished = true;
        setFeedback(QString(), tr("No puzzle available"));
        return;
    }
    m_view = *view;
    m_board->setFlipped(!m_view.whiteToMove);   // the player's side at the bottom
    m_board->setFen(m_view.fen);
    m_board->setLastMove(m_view.lastMove);
    emit puzzleChanged();
    refreshStats();
    setFeedback(QString(), m_view.whiteToMove ? tr("White to move: find the best move")
                                              : tr("Black to move: find the best move"));
}

void TrainerBridge::squareClicked(int row) {
    if (m_finished || m_waitingForReply || m_view.sessionId.isEmpty()) return;

    const bool own = m_board->isOwnPiece(row, m_view.whiteToMove);
    const int selected = m_board->selectedRow();

    if (selected < 0) {                    // nothing selected: only own pieces can be picked
        if (own) m_board->setSelectedRow(row);
        return;
    }
    if (row == selected) {                 // same square again: deselect
        m_board->setSelectedRow(-1);
        return;
    }
    if (own) {                             // another own piece: switch selection
        m_board->setSelectedRow(row);
        return;
    }

    QString uci = m_board->squareAt(selected) + m_board->squareAt(row);
    if (isPromotion(selected, row)) uci += QLatin1Char('q');   // auto-queen
    m_board->setSelectedRow(-1);
    playMove(uci);
}

bool TrainerBridge::setSelector(const QString& name) {
    const bool ok = m_service.setSelector(name);
    refreshStats();
    return ok;
}

void TrainerBridge::playMove(const QString& uci) {
    const MoveOutcome out = m_service.submitMove(m_view.sessionId, uci);
    if (!out.sessionFound) {
        m_finished = true;
        setFeedback(QString(), tr("This puzzle is no longer active. Press Next puzzle."));
        return;
    }

    switch (out.verdict) {
    case MoveVerdict::Illegal:
        setFeedback(QStringLiteral("illegal"), tr("Illegal move, try again"));
        return;
    case MoveVerdict::Wrong:
        m_finished = true;
        refreshStats();
        setFeedback(QStringLiteral("wrong"),
                    tr("Not the best move. Puzzle failed (rating %1).").arg(out.playerRating));
        return;
    case MoveVerdict::Correct:
        break;
    }

    m_board->setFen(out.fenAfterMove);
    m_board->setLastMove(uci);
    if (!out.opponentReply.isEmpty()) showReply(out.fen, out.opponentReply);

    if (out.solved) {
        m_finished = true;
        refreshStats();
        setFeedback(QStringLiteral("solved"), tr("Solved! Your rating: %1").arg(out.playerRating));
    } else {
        setFeedback(QStringLiteral("correct"), tr("Correct, keep going"));
    }
}

void TrainerBridge::showReply(const QString& fen, const QString& reply) {
    const int generation = m_generation;
    auto apply = [this, fen, reply, generation] {
        if (generation != m_generation) return;   // the player moved on to another puzzle
        m_board->setFen(fen);
        m_board->setLastMove(reply);
        m_waitingForReply = false;
    };
    if (m_replyDelayMs <= 0) {
        apply();
        return;
    }
    m_waitingForReply = true;
    QTimer::singleShot(m_replyDelayMs, this, apply);
}

bool TrainerBridge::isPromotion(int fromRow, int toRow) const {
    const QChar piece = m_board->pieceAt(fromRow);
    const QString target = m_board->squareAt(toRow);
    if (piece.toLower() != QLatin1Char('p') || target.size() != 2) return false;
    return target.at(1) == QLatin1Char('8') || target.at(1) == QLatin1Char('1');
}

void TrainerBridge::refreshStats() {
    m_stats = m_service.stats();
    emit statsChanged();
}

void TrainerBridge::setFeedback(const QString& kind, const QString& text) {
    m_feedback = kind;
    m_status = text;
    emit statusChanged();
}
