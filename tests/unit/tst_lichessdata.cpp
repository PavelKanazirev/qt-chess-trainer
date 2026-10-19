#include <QtTest>
#include <algorithm>
#include <climits>
#include "PuzzleRepository.h"
#include "PuzzleSession.h"

class TstLichessData : public QObject {
    Q_OBJECT
private slots:
    void everyPuzzleLoadsAndIsPlayable();
};

void TstLichessData::everyPuzzleLoadsAndIsPlayable() {
    PuzzleRepository repo;
    QString error;
    QVERIFY2(repo.loadFromFile(QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv"), &error),
             qPrintable(error));
    QVERIFY(repo.size() >= 150);
    QCOMPARE(repo.skippedCount(), 0);

    int minRating = INT_MAX;
    int maxRating = 0;
    for (const Puzzle& p : repo.all()) {
        QVERIFY2(PuzzleSession(p).isValid(), qPrintable(p.id));
        minRating = std::min(minRating, p.rating);
        maxRating = std::max(maxRating, p.rating);
    }
    QVERIFY(maxRating - minRating >= 1000);   // enough spread for ByRating
}

QTEST_APPLESS_MAIN(TstLichessData)
#include "tst_lichessdata.moc"
