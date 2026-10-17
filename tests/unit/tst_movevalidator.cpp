#include <QtTest>
#include "MoveValidator.h"

class TstMoveValidator : public QObject {
    Q_OBJECT
private slots:
    void expectedMoveIsCorrect();
    void illegalMoveIsIllegal();
    void legalNonMatingMoveIsWrong();
    void alternativeMateIsAccepted();
};

// Black has just played g8h8; white to find Ra8#
static ChessPosition backRank() {
    ChessPosition p(QStringLiteral("6k1/5ppp/8/8/8/8/5PPP/R5K1 b - - 0 1"));
    p.play(QStringLiteral("g8h8"));
    return p;
}

void TstMoveValidator::expectedMoveIsCorrect() {
    QCOMPARE(MoveValidator::check(backRank(), QStringLiteral("a1a8"), QStringLiteral("a1a8")),
             MoveVerdict::Correct);
}

void TstMoveValidator::illegalMoveIsIllegal() {
    QCOMPARE(MoveValidator::check(backRank(), QStringLiteral("a1b2"), QStringLiteral("a1a8")),
             MoveVerdict::Illegal);
}

void TstMoveValidator::legalNonMatingMoveIsWrong() {
    QCOMPARE(MoveValidator::check(backRank(), QStringLiteral("g1h1"), QStringLiteral("a1a8")),
             MoveVerdict::Wrong);
}

void TstMoveValidator::alternativeMateIsAccepted() {
    // Two rooks: both Ra8# and Re8# mate
    ChessPosition p(QStringLiteral("6k1/5ppp/8/8/8/8/5PPP/R3R1K1 b - - 0 1"));
    p.play(QStringLiteral("g8h8"));
    QCOMPARE(MoveValidator::check(p, QStringLiteral("e1e8"), QStringLiteral("a1a8")),
             MoveVerdict::Correct);
}

QTEST_APPLESS_MAIN(TstMoveValidator)
#include "tst_movevalidator.moc"
