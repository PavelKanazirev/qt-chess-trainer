#include <QtTest>
#include <atomic>
#include <thread>
#include <vector>
#include "PuzzleRepository.h"
#include "TrainerService.h"

class TstTrainerConcurrency : public QObject {
    Q_OBJECT
private slots:
    void parallelClientsKeepStatsConsistent();
};

void TstTrainerConcurrency::parallelClientsKeepStatsConsistent() {
    const QString path = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");
    PuzzleRepository solutions;   // read-only lookup of solution lines for the test
    QVERIFY(solutions.loadFromFile(path));

    TrainerService::Options o;
    o.dataPath = path;
    o.seed = 5;
    TrainerService svc(o);
    QVERIFY(svc.load());

    constexpr int kThreads = 8;
    constexpr int kRounds = 50;
    std::atomic<int> solvedByClients{0};

    std::vector<std::thread> clients;
    for (int t = 0; t < kThreads; ++t) {
        clients.emplace_back([&] {
            for (int r = 0; r < kRounds; ++r) {
                const auto view = svc.startNext();
                if (!view) continue;
                const Puzzle* p = solutions.findById(view->puzzleId);
                if (!p) continue;
                MoveOutcome out;
                for (int i = 1; i < p->moves.size(); i += 2)
                    out = svc.submitMove(view->sessionId, p->moves.at(i));
                if (out.solved) ++solvedByClients;
            }
        });
    }
    std::thread admin([&] {   // switches strategy while clients play
        const QStringList names = svc.availableSelectors();
        for (int i = 0; i < 200; ++i) svc.setSelector(names.at(i % names.size()));
    });

    for (auto& c : clients) c.join();
    admin.join();

    QCOMPARE(solvedByClients.load(), kThreads * kRounds);
    QCOMPARE(svc.stats().solved, kThreads * kRounds);
}

QTEST_APPLESS_MAIN(TstTrainerConcurrency)
#include "tst_trainerconcurrency.moc"
