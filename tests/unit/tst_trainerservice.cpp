#include <QtTest>
#include "TrainerService.h"

class TstTrainerService : public QObject {
    Q_OBJECT
private slots:
    void loadsPuzzles();
    void missingDataFailsToLoad();
    void startNextReturnsPlayablePosition();
    void switchesSelectorByName();
};

static TrainerService::Options sampleOptions() {
    TrainerService::Options o;
    o.dataPath = QStringLiteral(TRAINER_DATA_DIR "/puzzles_sample.csv");
    o.seed = 1;
    return o;
}

void TstTrainerService::loadsPuzzles() {
    TrainerService svc(sampleOptions());
    QString error;
    QVERIFY2(svc.load(&error), qPrintable(error));
    QCOMPARE(svc.puzzleCount(), 3);
}

void TstTrainerService::missingDataFailsToLoad() {
    auto o = sampleOptions();
    o.dataPath = QStringLiteral("/no/such/file.csv");
    TrainerService svc(o);
    QString error;
    QVERIFY(!svc.load(&error));
    QVERIFY(!error.isEmpty());
}

void TstTrainerService::startNextReturnsPlayablePosition() {
    TrainerService svc(sampleOptions());
    QVERIFY(svc.load());
    const auto view = svc.startNext();
    QVERIFY(view.has_value());
    QVERIFY(!view->sessionId.isEmpty());
    QVERIFY(!view->puzzleId.isEmpty());
    QVERIFY(!view->lastMove.isEmpty());
    QVERIFY(view->rating > 0);
}

void TstTrainerService::switchesSelectorByName() {
    TrainerService svc(sampleOptions());
    QCOMPARE(svc.selectorName(), QStringLiteral("ByRating"));
    QVERIFY(svc.setSelector(QStringLiteral("Random")));
    QCOMPARE(svc.selectorName(), QStringLiteral("Random"));
    QVERIFY(!svc.setSelector(QStringLiteral("Nope")));
    QCOMPARE(svc.selectorName(), QStringLiteral("Random"));   // unchanged
    QCOMPARE(svc.availableSelectors().size(), 3);
}

QTEST_APPLESS_MAIN(TstTrainerService)
#include "tst_trainerservice.moc"
