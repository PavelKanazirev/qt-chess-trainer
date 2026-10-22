#include "BoardModel.h"

namespace {

int squareFromName(const QString& name) {
    if (name.size() != 2) return -1;
    const int file = name.at(0).toLatin1() - 'a';
    const int rank = name.at(1).toLatin1() - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return rank * 8 + file;
}

QString glyphFor(QChar piece) {
    switch (piece.toLower().toLatin1()) {
    case 'k': return QString(QChar(0x265A));
    case 'q': return QString(QChar(0x265B));
    case 'r': return QString(QChar(0x265C));
    case 'b': return QString(QChar(0x265D));
    case 'n': return QString(QChar(0x265E));
    case 'p': return QString(QChar(0x265F));
    default: return {};
    }
}

}  // namespace

BoardModel::BoardModel(QObject* parent) : QAbstractListModel(parent) {
    m_pieces.fill(QChar());
}

int BoardModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 64;
}

int BoardModel::squareIndexForRow(int row) const {
    const int r = row / 8;
    const int c = row % 8;
    const int rank = m_flipped ? r : 7 - r;
    const int file = m_flipped ? 7 - c : c;
    return rank * 8 + file;
}

QString BoardModel::squareAt(int row) const {
    if (row < 0 || row > 63) return {};
    const int sq = squareIndexForRow(row);
    return QString(QChar('a' + sq % 8)) + QChar('1' + sq / 8);
}

int BoardModel::rowForSquare(const QString& square) const {
    const int sq = squareFromName(square);
    if (sq < 0) return -1;
    for (int row = 0; row < 64; ++row)
        if (squareIndexForRow(row) == sq) return row;
    return -1;
}

QVariant BoardModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() > 63) return {};
    const int sq = squareIndexForRow(index.row());
    const QChar piece = m_pieces[sq];

    switch (role) {
    case SquareNameRole: return squareAt(index.row());
    case PieceRole: return piece.isNull() ? QString() : QString(piece);
    case GlyphRole: return glyphFor(piece);
    case WhitePieceRole: return piece.isUpper();
    case LightRole: return (sq / 8 + sq % 8) % 2 == 1;   // a1 is a dark square
    case HighlightedRole: return sq == m_lastFrom || sq == m_lastTo;
    default: return {};
    }
}

QHash<int, QByteArray> BoardModel::roleNames() const {
    return {
        {SquareNameRole, "squareName"},
        {PieceRole, "piece"},
        {GlyphRole, "glyph"},
        {WhitePieceRole, "whitePiece"},
        {LightRole, "light"},
        {HighlightedRole, "highlighted"},
    };
}

bool BoardModel::setFen(const QString& fen) {
    const QStringList ranks = fen.section(QLatin1Char(' '), 0, 0).split(QLatin1Char('/'));
    if (ranks.size() != 8) return false;

    std::array<QChar, 64> pieces;
    pieces.fill(QChar());
    static const QString kPieces = QStringLiteral("kqrbnpKQRBNP");

    for (int i = 0; i < 8; ++i) {
        const int rank = 7 - i;
        int file = 0;
        for (const QChar c : ranks.at(i)) {
            if (c.isDigit()) {
                file += c.digitValue();
            } else {
                if (file > 7 || !kPieces.contains(c)) return false;
                pieces[rank * 8 + file] = c;
                ++file;
            }
        }
        if (file != 8) return false;
    }

    m_pieces = pieces;
    emitAllChanged();
    return true;
}

void BoardModel::setLastMove(const QString& uci) {
    m_lastFrom = uci.size() >= 4 ? squareFromName(uci.mid(0, 2)) : -1;
    m_lastTo = uci.size() >= 4 ? squareFromName(uci.mid(2, 2)) : -1;
    emitAllChanged();
}

void BoardModel::setFlipped(bool flipped) {
    if (m_flipped == flipped) return;
    m_flipped = flipped;
    emitAllChanged();
    emit flippedChanged();
}

void BoardModel::emitAllChanged() {
    emit dataChanged(index(0), index(63));   // row count never changes, so no reset needed
}
