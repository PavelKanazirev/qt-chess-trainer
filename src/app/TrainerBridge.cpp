#include "TrainerBridge.h"

TrainerBridge::TrainerBridge(TrainerService& service, QObject* parent)
    : QObject(parent), m_service(service), m_board(new BoardModel(this)) {
    refreshStats();
}

void TrainerBridge::nextPuzzle() {
    const auto view = m_service.startNext();
    if (!view) {
        setStatus(tr("No puzzle available"));
        return;
    }
    m_view = *view;
    m_board->setFlipped(!m_view.whiteToMove);   // the player's side at the bottom
    m_board->setFen(m_view.fen);
    m_board->setLastMove(m_view.lastMove);
    emit puzzleChanged();
    refreshStats();
    setStatus(m_view.whiteToMove ? tr("White to move: find the best move")
                                 : tr("Black to move: find the best move"));
}

void TrainerBridge::refreshStats() {
    m_stats = m_service.stats();
    emit statsChanged();
}

void TrainerBridge::setStatus(const QString& text) {
    if (m_status == text) return;
    m_status = text;
    emit statusChanged();
}
