#include "ChessPosition.h"

#include <chess.hpp>
#include <string>

struct ChessPosition::Impl {
    chess::Board board;

    bool findLegal(const std::string& uci, chess::Move& out) const {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        for (const auto& m : moves) {
            if (chess::uci::moveToUci(m) == uci) {
                out = m;
                return true;
            }
        }
        return false;
    }
};

ChessPosition::ChessPosition(const QString& fen) : d(std::make_unique<Impl>()) {
    d->board.setFen(fen.toStdString());
}

ChessPosition::ChessPosition(const ChessPosition& other)
    : d(std::make_unique<Impl>(*other.d)) {}

ChessPosition& ChessPosition::operator=(const ChessPosition& other) {
    if (this != &other) d = std::make_unique<Impl>(*other.d);
    return *this;
}

ChessPosition::ChessPosition(ChessPosition&&) noexcept = default;
ChessPosition& ChessPosition::operator=(ChessPosition&&) noexcept = default;
ChessPosition::~ChessPosition() = default;

bool ChessPosition::isLegal(const QString& uci) const {
    chess::Move m;
    return d->findLegal(uci.trimmed().toLower().toStdString(), m);
}

bool ChessPosition::play(const QString& uci) {
    chess::Move m;
    if (!d->findLegal(uci.trimmed().toLower().toStdString(), m)) return false;
    d->board.makeMove(m);
    return true;
}

bool ChessPosition::isCheckmate() const {
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, d->board);
    return moves.empty() && d->board.inCheck();
}

bool ChessPosition::whiteToMove() const {
    return d->board.sideToMove() == chess::Color::WHITE;
}

QString ChessPosition::fen() const {
    return QString::fromStdString(d->board.getFen());
}
