#include <QtTest>
#include "RandomSelector.h"
#include "ByRatingSelector.h"
#include "WeakestThemeSelector.h"

class TstSelectors : public QObject {
    Q_OBJECT
private slots:
    void emptyPoolReturnsNull();
    void randomAvoidsRecentPuzzles();
    void randomIsReproducibleWithSeed();
    void byRatingStaysInWindow();
    void byRatingFallsBackToClosest();
    void weakestThemeTargetsMostFailedTheme();
    void weakestThemeUsesFallbackWithoutData();
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

void TstSelectors::byRatingStaysInWindow() {
    QList<Puzzle> pool;
    for (int r = 800; r <= 2400; r += 100) pool.push_back(make(QString::number(r), r));
    SelectionContext ctx;
    ctx.playerRating = 1500;
    ctx.ratingWindow = 200;
    ByRatingSelector s(3);
    for (int i = 0; i < 50; ++i) {
        const Puzzle* p = s.select(pool, ctx);
        QVERIFY(p);
        QVERIFY2(std::abs(p->rating - 1500) <= 200, qPrintable(p->id));
    }
}

void TstSelectors::byRatingFallsBackToClosest() {
    const QList<Puzzle> pool{make("easy", 800), make("hard", 2000)};
    SelectionContext ctx;
    ctx.playerRating = 1500;
    ctx.ratingWindow = 100;
    ByRatingSelector s(3);
    QCOMPARE(s.select(pool, ctx)->id, QStringLiteral("hard"));   // 500 away vs 700
}

void TstSelectors::weakestThemeTargetsMostFailedTheme() {
    const QList<Puzzle> pool{make("pin1", 1500, {"pin"}),
                             make("fork1", 1600, {"fork"}),
                             make("fork2", 2400, {"fork"})};
    SelectionContext ctx;
    ctx.playerRating = 1500;
    ctx.themeFailures = {{QStringLiteral("fork"), 3}, {QStringLiteral("pin"), 1}};
    WeakestThemeSelector s(std::make_unique<RandomSelector>(1));
    QCOMPARE(s.select(pool, ctx)->id, QStringLiteral("fork1"));
}

void TstSelectors::weakestThemeUsesFallbackWithoutData() {
    const QList<Puzzle> pool{make("only", 1500, {"pin"})};
    WeakestThemeSelector s(std::make_unique<RandomSelector>(1));
    QCOMPARE(s.select(pool, SelectionContext{})->id, QStringLiteral("only"));
}

QTEST_APPLESS_MAIN(TstSelectors)
#include "tst_selectors.moc"
