#include <QtTest>
#include "TrainerBridge.h"

class TstTrainerBridge : public QObject {
    Q_OBJECT
private slots:
    void nextPuzzleFillsBoardAndProperties();
};

void TstTrainerBridge::nextPuzzleFillsBoardAndProperties() {
    TrainerService::Options o;
    o.dataPath = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");
    o.seed = 3;
    TrainerService service(o);
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
    QVERIFY(pieces >= 3);          // at least two kings and something to play with
    QCOMPARE(highlighted, 2);      // the opponent's last move
}

QTEST_APPLESS_MAIN(TstTrainerBridge)
#include "tst_trainerbridge.moc"
