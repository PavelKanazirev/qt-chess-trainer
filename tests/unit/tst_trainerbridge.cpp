#include <QtTest>
#include "PuzzleRepository.h"
#include "TrainerBridge.h"

class TstTrainerBridge : public QObject {
    Q_OBJECT
private slots:
    void nextPuzzleFillsBoardAndProperties();
    void selectionRules();
    void clickingTheSolutionSolvesThePuzzle();
    void switchesSelector();
};

static TrainerService::Options lichessOptions(quint32 seed) {
    TrainerService::Options o;
    o.dataPath = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");
    o.seed = seed;
    return o;
}

void TstTrainerBridge::nextPuzzleFillsBoardAndProperties() {
    TrainerService service(lichessOptions(3));
    QVERIFY(service.load());

    TrainerBridge bridge(service);
    QSignalSpy puzzleSpy(&bridge, &TrainerBridge::puzzleChanged);
    bridge.nextPuzzle();

    QCOMPARE(puzzleSpy.count(), 1);
    QVERIFY(!bridge.puzzleId().isEmpty());
    QVERIFY(bridge.puzzleRating() > 0);
    QVERIFY(!bridge.statusText().isEmpty());
    QCOMPARE(bridge.board()->flipped(), !bridge.whiteToMove());

    int pieces = 0;
    int highlighted = 0;
    for (int row = 0; row < 64; ++row) {
        const QModelIndex i = bridge.board()->index(row);
        if (!i.data(BoardModel::PieceRole).toString().isEmpty()) ++pieces;
        if (i.data(BoardModel::HighlightedRole).toBool()) ++highlighted;
    }
    QVERIFY(pieces >= 3);
    QCOMPARE(highlighted, 2);
}

void TstTrainerBridge::selectionRules() {
    TrainerService service(lichessOptions(4));
    QVERIFY(service.load());
    TrainerBridge bridge(service);
    bridge.nextPuzzle();
    BoardModel* b = bridge.board();

    int own = -1;
    int empty = -1;
    for (int row = 0; row < 64; ++row) {
        if (own < 0 && b->isOwnPiece(row, bridge.whiteToMove())) own = row;
        if (empty < 0 && b->pieceAt(row).isNull()) empty = row;
    }
    QVERIFY(own >= 0 && empty >= 0);

    bridge.squareClicked(empty);           // nothing to select
    QCOMPARE(b->selectedRow(), -1);
    bridge.squareClicked(own);
    QCOMPARE(b->selectedRow(), own);
    bridge.squareClicked(own);             // deselect
    QCOMPARE(b->selectedRow(), -1);
}

void TstTrainerBridge::clickingTheSolutionSolvesThePuzzle() {
    PuzzleRepository solutions;
    QVERIFY(solutions.loadFromFile(QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv")));

    TrainerService service(lichessOptions(8));
    QVERIFY(service.load());
    TrainerBridge bridge(service);
    bridge.setReplyDelay(0);

    const Puzzle* p = nullptr;
    for (int attempt = 0; attempt < 10 && !p; ++attempt) {   // skip under-promotions
        bridge.nextPuzzle();
        const Puzzle* candidate = solutions.findById(bridge.puzzleId());
        QVERIFY(candidate);
        bool underPromotion = false;
        for (const QString& m : candidate->moves)
            if (m.size() == 5 && m.at(4) != QLatin1Char('q')) underPromotion = true;
        if (!underPromotion) p = candidate;
    }
    QVERIFY(p);

    BoardModel* b = bridge.board();
    for (int i = 1; i < p->moves.size(); i += 2) {
        const QString move = p->moves.at(i);
        bridge.squareClicked(b->rowForSquare(move.mid(0, 2)));
        bridge.squareClicked(b->rowForSquare(move.mid(2, 2)));
        QVERIFY2(bridge.feedback() == QLatin1String("correct")
                     || bridge.feedback() == QLatin1String("solved"),
                 qPrintable(QStringLiteral("%1 at move %2: %3")
                                .arg(p->id, move, bridge.statusText())));
    }
    QCOMPARE(bridge.feedback(), QStringLiteral("solved"));
    QVERIFY(bridge.finished());
    QCOMPARE(bridge.solvedCount(), 1);
}

void TstTrainerBridge::switchesSelector() {
    TrainerService service(lichessOptions(1));
    TrainerBridge bridge(service);
    QSignalSpy spy(&bridge, &TrainerBridge::statsChanged);
    QVERIFY(bridge.setSelector(QStringLiteral("Random")));
    QCOMPARE(bridge.selector(), QStringLiteral("Random"));
    QVERIFY(!bridge.setSelector(QStringLiteral("Nope")));
    QCOMPARE(bridge.selector(), QStringLiteral("Random"));
    QVERIFY(spy.count() >= 1);
    QCOMPARE(bridge.availableSelectors().size(), 3);
}

QTEST_APPLESS_MAIN(TstTrainerBridge)
#include "tst_trainerbridge.moc"
