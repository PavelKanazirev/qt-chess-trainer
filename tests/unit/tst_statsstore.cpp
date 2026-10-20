#include <QtTest>
#include "StatsStore.h"

class TstStatsStore : public QObject {
    Q_OBJECT
private slots:
    void solvingEqualPuzzleRaisesRating();
    void failingEqualPuzzleLowersRating();
    void themeFailuresReachSelectionContext();
    void recentIdsAreCappedAndMoveToEnd();
    void resetRestoresInitialState();
};

static Puzzle make(const QString& id, int rating, const QStringList& themes = {}) {
    Puzzle p;
    p.id = id;
    p.rating = rating;
    p.themes = themes;
    return p;
}

void TstStatsStore::solvingEqualPuzzleRaisesRating() {
    StatsStore s(1500);
    s.record(make("a", 1500), true);
    QCOMPARE(s.playerRating(), 1516);   // expected score 0.5, K = 32
    QCOMPARE(s.solvedCount(), 1);
}

void TstStatsStore::failingEqualPuzzleLowersRating() {
    StatsStore s(1500);
    s.record(make("a", 1500), false);
    QCOMPARE(s.playerRating(), 1484);
    QCOMPARE(s.failedCount(), 1);
}

void TstStatsStore::themeFailuresReachSelectionContext() {
    StatsStore s;
    s.record(make("a", 1500, {"fork"}), false);
    s.record(make("b", 1500, {"fork"}), false);
    s.record(make("c", 1500, {"pin"}), true);
    const SelectionContext ctx = s.selectionContext(150);
    QCOMPARE(ctx.ratingWindow, 150);
    QCOMPARE(ctx.themeFailures.value(QStringLiteral("fork")), 2);
    QVERIFY(!ctx.themeFailures.contains(QStringLiteral("pin")));   // no failures there
    QCOMPARE(ctx.recentIds.size(), 3);
}

void TstStatsStore::recentIdsAreCappedAndMoveToEnd() {
    StatsStore s(1500, 3);
    for (const char* id : {"a", "b", "c", "d"}) s.record(make(id, 1500), true);
    QCOMPARE(s.recentIds(), (QList<QString>{"b", "c", "d"}));
    s.record(make("b", 1500), true);
    QCOMPARE(s.recentIds(), (QList<QString>{"c", "d", "b"}));
}

void TstStatsStore::resetRestoresInitialState() {
    StatsStore s(1200);
    s.record(make("a", 1500, {"fork"}), false);
    s.reset();
    QCOMPARE(s.playerRating(), 1200);
    QCOMPARE(s.failedCount(), 0);
    QVERIFY(s.themes().isEmpty());
    QVERIFY(s.recentIds().isEmpty());
}

QTEST_APPLESS_MAIN(TstStatsStore)
#include "tst_statsstore.moc"
