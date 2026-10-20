#include <QtTest>
#include <cstdlib>
#include "PuzzleRepository.h"
#include "TrainerService.h"

class TstTrainerFlow : public QObject {
    Q_OBJECT
private slots:
    void thirtySolvedPuzzlesWithByRating();
};

void TstTrainerFlow::thirtySolvedPuzzlesWithByRating() {
    const QString path = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");
    PuzzleRepository solutions;
    QVERIFY(solutions.loadFromFile(path));

    TrainerService::Options o;
    o.dataPath = path;
    o.seed = 11;
    o.ratingWindow = 200;
    TrainerService svc(o);
    QVERIFY(svc.load());

    QList<QString> history;
    int lastRating = svc.stats().playerRating;
    for (int round = 0; round < 30; ++round) {
        const auto view = svc.startNext();
        QVERIFY(view);
        QVERIFY2(std::abs(view->rating - lastRating) <= 200,
                 qPrintable(QStringLiteral("puzzle %1 rated %2, player %3")
                                .arg(view->puzzleId).arg(view->rating).arg(lastRating)));
        QVERIFY2(!history.mid(std::max(0, int(history.size()) - 20)).contains(view->puzzleId),
                 "puzzle repeated within the last 20");
        history.push_back(view->puzzleId);

        const Puzzle* p = solutions.findById(view->puzzleId);
        QVERIFY(p);
        MoveOutcome out;
        for (int i = 1; i < p->moves.size(); i += 2)
            out = svc.submitMove(view->sessionId, p->moves.at(i));
        QVERIFY(out.solved);
        QVERIFY(out.playerRating > lastRating);   // every solve raises the estimate
        lastRating = out.playerRating;
    }

    const StatsSnapshot s = svc.stats();
    QCOMPARE(s.solved, 30);
    QCOMPARE(s.failed, 0);
    QVERIFY(!s.themes.isEmpty());
}

QTEST_APPLESS_MAIN(TstTrainerFlow)
#include "tst_trainerflow.moc"
