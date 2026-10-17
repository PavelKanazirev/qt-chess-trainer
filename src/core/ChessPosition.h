#pragma once
#include <memory>
#include <QString>

// Adapter around the chess library. Only ChessPosition.cpp includes chess.hpp.
class ChessPosition {
public:
    explicit ChessPosition(const QString& fen);
    ChessPosition(const ChessPosition& other);
    ChessPosition& operator=(const ChessPosition& other);
    ChessPosition(ChessPosition&&) noexcept;
    ChessPosition& operator=(ChessPosition&&) noexcept;
    ~ChessPosition();

    bool isLegal(const QString& uci) const;
    bool play(const QString& uci);     // false (and no change) if the move is illegal
    bool isCheckmate() const;
    bool whiteToMove() const;
    QString fen() const;

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
