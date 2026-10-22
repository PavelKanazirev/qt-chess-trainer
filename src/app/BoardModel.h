#pragma once
#include <QAbstractListModel>
#include <array>

// The 64 squares in display order: row 0 is the top-left square as seen by the player.
class BoardModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool flipped READ flipped WRITE setFlipped NOTIFY flippedChanged)

public:
    enum Role {
        SquareNameRole = Qt::UserRole + 1,
        PieceRole,
        GlyphRole,
        WhitePieceRole,
        LightRole,
        HighlightedRole,
    };

    explicit BoardModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool setFen(const QString& fen);          // false (no change) if the placement is malformed
    void setLastMove(const QString& uci);     // highlights from/to; empty string clears
    bool flipped() const { return m_flipped; }
    void setFlipped(bool flipped);            // true: black at the bottom

    QString squareAt(int row) const;          // "e4"; empty if row is out of range
    int rowForSquare(const QString& square) const;   // -1 if invalid

signals:
    void flippedChanged();

private:
    int squareIndexForRow(int row) const;     // 0 = a1 ... 63 = h8
    void emitAllChanged();

    std::array<QChar, 64> m_pieces{};
    int m_lastFrom = -1;
    int m_lastTo = -1;
    bool m_flipped = false;
};
