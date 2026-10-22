#include <QAbstractItemModelTester>
#include <QtTest>
#include "BoardModel.h"

class TstBoardModel : public QObject {
    Q_OBJECT
private slots:
    void startingPositionLayout();
    void flippedOrientation();
    void rejectsMalformedFen();
    void highlightsLastMove();
    void obeysModelContract();
};

static const QString kStart =
    QStringLiteral("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

static QVariant at(const BoardModel& m, int row, int role) {
    return m.data(m.index(row), role);
}

void TstBoardModel::startingPositionLayout() {
    BoardModel m;
    QVERIFY(m.setFen(kStart));
    QCOMPARE(m.rowCount(), 64);
    QCOMPARE(at(m, 0, BoardModel::SquareNameRole).toString(), QStringLiteral("a8"));
    QCOMPARE(at(m, 0, BoardModel::PieceRole).toString(), QStringLiteral("r"));
    QCOMPARE(at(m, 63, BoardModel::SquareNameRole).toString(), QStringLiteral("h1"));
    QCOMPARE(at(m, 63, BoardModel::PieceRole).toString(), QStringLiteral("R"));
    QVERIFY(at(m, 63, BoardModel::WhitePieceRole).toBool());
    QVERIFY(at(m, m.rowForSquare("e4"), BoardModel::PieceRole).toString().isEmpty());
    QVERIFY(!at(m, m.rowForSquare("a1"), BoardModel::LightRole).toBool());   // a1 is dark
    QVERIFY(at(m, m.rowForSquare("h1"), BoardModel::LightRole).toBool());
}

void TstBoardModel::flippedOrientation() {
    BoardModel m;
    QVERIFY(m.setFen(kStart));
    QSignalSpy spy(&m, &BoardModel::flippedChanged);
    m.setFlipped(true);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(m.squareAt(0), QStringLiteral("h1"));
    QCOMPARE(m.squareAt(63), QStringLiteral("a8"));
    QCOMPARE(at(m, 0, BoardModel::PieceRole).toString(), QStringLiteral("R"));
}

void TstBoardModel::rejectsMalformedFen() {
    BoardModel m;
    QVERIFY(m.setFen(kStart));
    QVERIFY(!m.setFen(QStringLiteral("8/8 w - - 0 1")));
    QVERIFY(!m.setFen(QStringLiteral("9/8/8/8/8/8/8/8 w - - 0 1")));
    QVERIFY(!m.setFen(QStringLiteral("x7/8/8/8/8/8/8/8 w - - 0 1")));
    QCOMPARE(at(m, 0, BoardModel::PieceRole).toString(), QStringLiteral("r"));   // unchanged
}

void TstBoardModel::highlightsLastMove() {
    BoardModel m;
    QVERIFY(m.setFen(kStart));
    m.setLastMove(QStringLiteral("e2e4"));
    int highlighted = 0;
    for (int row = 0; row < 64; ++row)
        if (at(m, row, BoardModel::HighlightedRole).toBool()) ++highlighted;
    QCOMPARE(highlighted, 2);
    QVERIFY(at(m, m.rowForSquare("e2"), BoardModel::HighlightedRole).toBool());
    QVERIFY(at(m, m.rowForSquare("e4"), BoardModel::HighlightedRole).toBool());
    m.setLastMove(QString());
    QVERIFY(!at(m, m.rowForSquare("e2"), BoardModel::HighlightedRole).toBool());
}

void TstBoardModel::obeysModelContract() {
    BoardModel m;
    QAbstractItemModelTester tester(&m, QAbstractItemModelTester::FailureReportingMode::QtTest);
    QVERIFY(m.setFen(kStart));
    m.setFlipped(true);
    m.setLastMove(QStringLiteral("g1f3"));
}

QTEST_APPLESS_MAIN(TstBoardModel)
#include "tst_boardmodel.moc"
