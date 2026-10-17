#include <QtTest>
#include "PuzzleRepository.h"
#include "PuzzleSession.h"

class TstPuzzleSession : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void opponentMovesFirst();
    void solvesMateInOne();
    void wrongMoveFailsThePuzzle();
    void illegalMoveKeepsSessionOpen();
    void multiMovePuzzleAutoPlaysReplies();
    void malformedPuzzleIsInvalid();

private:
    Puzzle sample(const QString& id) const;
    PuzzleRepository m_repo;
};

void TstPuzzleSession::initTestCase() {
    QString error;
    QVERIFY2(m_repo.loadFromFile(QStringLiteral(TRAINER_DATA_DIR "/puzzles_sample.csv"), &error),
             qPrintable(error));
}

Puzzle TstPuzzleSession::sample(const QString& id) const {
    const Puzzle* p = m_repo.findById(id);
    return p ? *p : Puzzle{};
}

void TstPuzzleSession::opponentMovesFirst() {
    PuzzleSession s(sample(QStringLiteral("t003")));
    QVERIFY(s.isValid());
    QCOMPARE(s.firstOpponentMove(), QStringLiteral("g2g4"));
    QVERIFY(s.fen().contains(QStringLiteral(" b ")));   // black (the player) to move
}

void TstPuzzleSession::solvesMateInOne() {
    PuzzleSession s(sample(QStringLiteral("t003")));
    const MoveResult r = s.submit(QStringLiteral("d8h4"));
    QCOMPARE(r.verdict, MoveVerdict::Correct);
    QVERIFY(r.solved);
    QVERIFY(s.isSolved());
}

void TstPuzzleSession::wrongMoveFailsThePuzzle() {
    PuzzleSession s(sample(QStringLiteral("t003")));
    QCOMPARE(s.submit(QStringLiteral("a7a6")).verdict, MoveVerdict::Wrong);
    QVERIFY(s.isFailed());
    QCOMPARE(s.submit(QStringLiteral("d8h4")).verdict, MoveVerdict::Illegal);  // already over
    QVERIFY(!s.isSolved());
}

void TstPuzzleSession::illegalMoveKeepsSessionOpen() {
    PuzzleSession s(sample(QStringLiteral("t003")));
    QCOMPARE(s.submit(QStringLiteral("d8d4")).verdict, MoveVerdict::Illegal);   // blocked by pawn
    QVERIFY(!s.isFinished());
    QVERIFY(s.submit(QStringLiteral("d8h4")).solved);
}

void TstPuzzleSession::multiMovePuzzleAutoPlaysReplies() {
    Puzzle p;
    p.id = QStringLiteral("multi");
    p.fen = QStringLiteral("rnbqkbnr/pppp1ppp/8/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2");
    p.moves = {QStringLiteral("b8c6"), QStringLiteral("d1h5"),
               QStringLiteral("g8f6"), QStringLiteral("h5f7")};
    p.rating = 800;

    PuzzleSession s(p);
    QVERIFY(s.isValid());

    const MoveResult first = s.submit(QStringLiteral("d1h5"));
    QCOMPARE(first.verdict, MoveVerdict::Correct);
    QCOMPARE(first.opponentReply, QStringLiteral("g8f6"));
    QVERIFY(!first.solved);

    const MoveResult second = s.submit(QStringLiteral("h5f7"));
    QCOMPARE(second.verdict, MoveVerdict::Correct);
    QVERIFY(second.solved);
}

void TstPuzzleSession::malformedPuzzleIsInvalid() {
    Puzzle p = sample(QStringLiteral("t003"));
    p.moves = {QStringLiteral("g2g4")};                       // too short
    QVERIFY(!PuzzleSession(p).isValid());

    p.moves = {QStringLiteral("g2g5"), QStringLiteral("d8h4")};   // opponent move is illegal
    QVERIFY(!PuzzleSession(p).isValid());
}

QTEST_APPLESS_MAIN(TstPuzzleSession)
#include "tst_puzzlesession.moc"
