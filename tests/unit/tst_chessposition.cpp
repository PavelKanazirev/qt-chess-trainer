#include <QtTest>
#include "ChessPosition.h"

class TstChessPosition : public QObject {
    Q_OBJECT
private slots:
    void legalAndIllegalMoves();
    void detectsCheckmate();
    void copiesAreIndependent();
};

static const QString kStart =
    QStringLiteral("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

void TstChessPosition::legalAndIllegalMoves() {
    ChessPosition pos(kStart);
    QVERIFY(pos.whiteToMove());
    QVERIFY(pos.isLegal(QStringLiteral("e2e4")));
    QVERIFY(!pos.isLegal(QStringLiteral("e2e5")));   // pawn cannot jump three squares
    QVERIFY(!pos.isLegal(QStringLiteral("e7e5")));   // black piece, white to move
    QVERIFY(!pos.play(QStringLiteral("e2e5")));
    QVERIFY(pos.play(QStringLiteral("e2e4")));
    QVERIFY(!pos.whiteToMove());
}

void TstChessPosition::detectsCheckmate() {
    // Fool's mate
    ChessPosition pos(QStringLiteral(
        "rnbqkbnr/pppp1ppp/8/4p3/8/5P2/PPPPP1PP/RNBQKBNR w KQkq - 0 2"));
    QVERIFY(!pos.isCheckmate());
    QVERIFY(pos.play(QStringLiteral("g2g4")));
    QVERIFY(!pos.isCheckmate());
    QVERIFY(pos.play(QStringLiteral("d8h4")));
    QVERIFY(pos.isCheckmate());
}

void TstChessPosition::copiesAreIndependent() {
    ChessPosition a(kStart);
    ChessPosition b = a;
    QVERIFY(b.play(QStringLiteral("e2e4")));
    QVERIFY(a.whiteToMove());
    QVERIFY(!b.whiteToMove());
    QCOMPARE(a.fen(), kStart);
}

QTEST_APPLESS_MAIN(TstChessPosition)
#include "tst_chessposition.moc"
