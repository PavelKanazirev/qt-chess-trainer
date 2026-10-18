#include <QtTest>
#include "RandomSelector.h"

class TstSelectors : public QObject {
    Q_OBJECT
private slots:
    void emptyPoolReturnsNull();
    void randomAvoidsRecentPuzzles();
    void randomIsReproducibleWithSeed();
};

static Puzzle make(const QString& id, int rating, const QStringList& themes = {}) {
    Puzzle p;
    p.id = id;
    p.rating = rating;
    p.themes = themes;
    return p;
}

void TstSelectors::emptyPoolReturnsNull() {
    RandomSelector s(1);
    QVERIFY(s.select({}, SelectionContext{}) == nullptr);
}

void TstSelectors::randomAvoidsRecentPuzzles() {
    const QList<Puzzle> pool{make("a", 1000), make("b", 1200), make("c", 1400)};
    SelectionContext ctx;
    ctx.recentIds = {QStringLiteral("a"), QStringLiteral("b")};
    RandomSelector s(7);
    for (int i = 0; i < 20; ++i)
        QCOMPARE(s.select(pool, ctx)->id, QStringLiteral("c"));
}

void TstSelectors::randomIsReproducibleWithSeed() {
    QList<Puzzle> pool;
    for (int i = 0; i < 50; ++i) pool.push_back(make(QString::number(i), 1000 + i));
    RandomSelector a(42);
    RandomSelector b(42);
    for (int i = 0; i < 10; ++i)
        QCOMPARE(a.select(pool, {})->id, b.select(pool, {})->id);
}

QTEST_APPLESS_MAIN(TstSelectors)
#include "tst_selectors.moc"
