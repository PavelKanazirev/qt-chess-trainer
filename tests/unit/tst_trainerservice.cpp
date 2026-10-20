#include <QtTest>
#include "TrainerService.h"
#include "PuzzleRepository.h"

class TstTrainerService : public QObject {
    Q_OBJECT
private slots:
    void loadsPuzzles();
    void missingDataFailsToLoad();
    void startNextReturnsPlayablePosition();
    void switchesSelectorByName();
    void solvingUpdatesStatsAndClosesSession();
    void wrongMoveFailsPuzzle();
    void illegalMoveKeepsSessionOpen();
    void unknownSessionIsReported();
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

static Puzzle samplePuzzle(const QString& id) {
    PuzzleRepository repo;
    repo.loadFromFile(QStringLiteral(TRAINER_DATA_DIR "/puzzles_sample.csv"));
    const Puzzle* p = repo.findById(id);
    return p ? *p : Puzzle{};
}

void TstTrainerService::solvingUpdatesStatsAndClosesSession() {
    TrainerService svc(sampleOptions());
    QVERIFY(svc.load());
    const auto view = svc.startNext();
    QVERIFY(view);
    const Puzzle p = samplePuzzle(view->puzzleId);

    MoveOutcome out;
    for (int i = 1; i < p.moves.size(); i += 2)   // player moves; replies are auto-played
        out = svc.submitMove(view->sessionId, p.moves.at(i));

    QVERIFY(out.solved);
    QCOMPARE(svc.stats().solved, 1);
    QVERIFY(svc.stats().playerRating >= 1500);
    QVERIFY(!svc.submitMove(view->sessionId, p.moves.at(1)).sessionFound);   // closed
}

void TstTrainerService::wrongMoveFailsPuzzle() {
    const QHash<QString, QString> wrongMove{{"t001", "g1f1"}, {"t002", "a2a3"}, {"t003", "a7a6"}};
    TrainerService svc(sampleOptions());
    QVERIFY(svc.load());
    const auto view = svc.startNext();
    QVERIFY(view);

    const MoveOutcome out = svc.submitMove(view->sessionId, wrongMove.value(view->puzzleId));
    QCOMPARE(out.verdict, MoveVerdict::Wrong);
    QVERIFY(out.failed);
    QCOMPARE(svc.stats().failed, 1);
    QVERIFY(svc.stats().playerRating < 1500);
}

void TstTrainerService::illegalMoveKeepsSessionOpen() {
    TrainerService svc(sampleOptions());
    QVERIFY(svc.load());
    const auto view = svc.startNext();
    QVERIFY(view);
    QCOMPARE(svc.submitMove(view->sessionId, QStringLiteral("a1a1")).verdict, MoveVerdict::Illegal);
    QVERIFY(svc.submitMove(view->sessionId, QStringLiteral("a1a1")).sessionFound);
    QCOMPARE(svc.stats().solved + svc.stats().failed, 0);
}

void TstTrainerService::unknownSessionIsReported() {
    TrainerService svc(sampleOptions());
    QVERIFY(svc.load());
    QVERIFY(!svc.submitMove(QStringLiteral("nope"), QStringLiteral("e2e4")).sessionFound);
}

QTEST_APPLESS_MAIN(TstTrainerService)
#include "tst_trainerservice.moc"
