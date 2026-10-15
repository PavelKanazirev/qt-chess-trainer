#include <QtTest>
#include "PuzzleParser.h"

class TstPuzzleParser : public QObject {
    Q_OBJECT
private slots:
    void parsesValidLine();
    void rejectsMalformedLines_data();
    void rejectsMalformedLines();
};

void TstPuzzleParser::parsesValidLine() {
    const QString line = QStringLiteral(
        "t001,6k1/5ppp/8/8/8/8/5PPP/R5K1 b - - 0 1,g8h8 a1a8,600,75,90,1000,"
        "backRankMate mateIn1 short,,");
    const auto p = PuzzleParser::parseLine(line);
    QVERIFY(p.has_value());
    QCOMPARE(p->id, QStringLiteral("t001"));
    QCOMPARE(p->moves, (QStringList{QStringLiteral("g8h8"), QStringLiteral("a1a8")}));
    QCOMPARE(p->rating, 600);
    QVERIFY(p->themes.contains(QStringLiteral("mateIn1")));
}

void TstPuzzleParser::rejectsMalformedLines_data() {
    QTest::addColumn<QString>("line");
    const QString fen = QStringLiteral("6k1/5ppp/8/8/8/8/5PPP/R5K1 b - - 0 1");
    QTest::newRow("too few fields") << QStringLiteral("a,b,c");
    QTest::newRow("empty id") << QStringLiteral(",%1,g8h8 a1a8,600,0,0,0,x,,").arg(fen);
    QTest::newRow("bad fen") << QStringLiteral("t,notafen,g8h8 a1a8,600,0,0,0,x,,");
    QTest::newRow("one move") << QStringLiteral("t,%1,g8h8,600,0,0,0,x,,").arg(fen);
    QTest::newRow("bad move") << QStringLiteral("t,%1,g8h9 a1a8,600,0,0,0,x,,").arg(fen);
    QTest::newRow("bad rating") << QStringLiteral("t,%1,g8h8 a1a8,abc,0,0,0,x,,").arg(fen);
}

void TstPuzzleParser::rejectsMalformedLines() {
    QFETCH(QString, line);
    QVERIFY(!PuzzleParser::parseLine(line).has_value());
}

QTEST_APPLESS_MAIN(TstPuzzleParser)
#include "tst_puzzleparser.moc"
