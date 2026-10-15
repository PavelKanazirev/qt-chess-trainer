#include <QtTest>
#include "PuzzleRepository.h"

class TstPuzzleRepository : public QObject {
    Q_OBJECT
private slots:
    void loadsSampleFile();
    void missingFileFails();
};

static QString samplePath() {
    return QStringLiteral(TRAINER_DATA_DIR) + QStringLiteral("/puzzles_sample.csv");
}

void TstPuzzleRepository::loadsSampleFile() {
    PuzzleRepository repo;
    QString error;
    QVERIFY2(repo.loadFromFile(samplePath(), &error), qPrintable(error));
    QCOMPARE(repo.size(), 3);
    QCOMPARE(repo.skippedCount(), 0);

    const Puzzle* p = repo.findById(QStringLiteral("t002"));
    QVERIFY(p != nullptr);
    QCOMPARE(p->rating, 500);
    QVERIFY(repo.findById(QStringLiteral("missing")) == nullptr);
}

void TstPuzzleRepository::missingFileFails() {
    PuzzleRepository repo;
    QString error;
    QVERIFY(!repo.loadFromFile(QStringLiteral("/no/such/file.csv"), &error));
    QVERIFY(!error.isEmpty());
}

QTEST_APPLESS_MAIN(TstPuzzleRepository)
#include "tst_puzzlerepository.moc"
