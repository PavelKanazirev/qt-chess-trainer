#include <QtTest>
#include <QJsonArray>
#include "ApiJson.h"

class TstApiJson : public QObject {
    Q_OBJECT
private slots:
    void puzzleViewToJson();
    void moveOutcomeToJson();
    void statsIncludeThemes();
    void moveFromBodyAcceptsUci();
    void moveFromBodyRejectsBadInput_data();
    void moveFromBodyRejectsBadInput();
};

void TstApiJson::puzzleViewToJson() {
    PuzzleView v;
    v.sessionId = QStringLiteral("s1");
    v.puzzleId = QStringLiteral("p1");
    v.fen = QStringLiteral("8/8/8/8/8/8/8/K6k b - - 0 1");
    v.lastMove = QStringLiteral("a2a1");
    v.rating = 1234;
    v.themes = {QStringLiteral("fork"), QStringLiteral("short")};
    v.whiteToMove = false;

    const QJsonObject o = ApiJson::puzzle(v);
    QCOMPARE(o.value("sessionId").toString(), QStringLiteral("s1"));
    QCOMPARE(o.value("rating").toInt(), 1234);
    QCOMPARE(o.value("sideToMove").toString(), QStringLiteral("black"));
    QCOMPARE(o.value("themes").toArray().size(), 2);
}

void TstApiJson::moveOutcomeToJson() {
    MoveOutcome out;
    out.sessionFound = true;
    out.verdict = MoveVerdict::Wrong;
    out.failed = true;
    out.playerRating = 1480;
    const QJsonObject o = ApiJson::moveOutcome(out);
    QCOMPARE(o.value("verdict").toString(), QStringLiteral("wrong"));
    QVERIFY(o.value("failed").toBool());
    QVERIFY(!o.value("solved").toBool());
    QCOMPARE(o.value("playerRating").toInt(), 1480);
    QCOMPARE(ApiJson::verdictName(MoveVerdict::Illegal), QStringLiteral("illegal"));
}

void TstApiJson::statsIncludeThemes() {
    StatsSnapshot s;
    s.playerRating = 1600;
    s.solved = 3;
    s.selector = QStringLiteral("ByRating");
    s.themes.insert(QStringLiteral("fork"), ThemeStats{2, 1});
    const QJsonObject o = ApiJson::stats(s);
    QCOMPARE(o.value("solved").toInt(), 3);
    QCOMPARE(o.value("selector").toString(), QStringLiteral("ByRating"));
    QCOMPARE(o.value("themes").toObject().value("fork").toObject().value("failed").toInt(), 1);
}

void TstApiJson::moveFromBodyAcceptsUci() {
    QCOMPARE(ApiJson::moveFromBody(R"({"move":"e2e4"})").value(), QStringLiteral("e2e4"));
    QCOMPARE(ApiJson::moveFromBody(R"({"move":" e7e8q "})").value(), QStringLiteral("e7e8q"));
}

void TstApiJson::moveFromBodyRejectsBadInput_data() {
    QTest::addColumn<QString>("body");
    QTest::newRow("empty") << QString();
    QTest::newRow("not json") << QStringLiteral("move=e2e4");
    QTest::newRow("array") << QStringLiteral(R"(["e2e4"])");
    QTest::newRow("no move") << QStringLiteral("{}");
    QTest::newRow("number") << QStringLiteral(R"({"move":5})");
    QTest::newRow("too short") << QStringLiteral(R"({"move":"e2"})");
    QTest::newRow("too long") << QStringLiteral(R"({"move":"e2e4e5"})");
}

void TstApiJson::moveFromBodyRejectsBadInput() {
    QFETCH(QString, body);
    QVERIFY(!ApiJson::moveFromBody(body.toStdString()).has_value());
}

QTEST_APPLESS_MAIN(TstApiJson)
#include "tst_apijson.moc"
