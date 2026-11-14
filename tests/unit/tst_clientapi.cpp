#include <httplib.h>   // first, before any Qt header

#include <QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>

#include "ClientApiServer.h"
#include "PuzzleRepository.h"
#include "TrainerService.h"

class TstClientApi : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void healthReportsPuzzles();
    void statsStartEmpty();
    void unknownRouteIs404();
    void solvingThroughTheApi();
    void badBodyIs400();
    void unknownSessionIs404();
    void cleanupTestCase();

private:
    std::unique_ptr<TrainerService> m_service;
    std::unique_ptr<ClientApiServer> m_server;
    PuzzleRepository m_solutions;
};

static QJsonObject jsonBody(const httplib::Result& res) {
    return QJsonDocument::fromJson(QByteArray::fromStdString(res->body)).object();
}

void TstClientApi::initTestCase() {
    const QString path = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");
    QVERIFY(m_solutions.loadFromFile(path));

    TrainerService::Options o;
    o.dataPath = path;
    o.seed = 21;
    m_service = std::make_unique<TrainerService>(o);
    QVERIFY(m_service->load());

    ClientApiServer::Options so;
    so.port = 0;   // any free port
    m_server = std::make_unique<ClientApiServer>(*m_service, so);
    QString error;
    QVERIFY2(m_server->start(&error), qPrintable(error));
    QVERIFY(m_server->port() > 0);
}

void TstClientApi::healthReportsPuzzles() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Get("/health");
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    const QJsonObject body = jsonBody(res);
    QCOMPARE(body.value("status").toString(), QStringLiteral("ok"));
    QCOMPARE(body.value("puzzles").toInt(), m_service->puzzleCount());
}

void TstClientApi::statsStartEmpty() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Get("/stats");
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    QCOMPARE(jsonBody(res).value("solved").toInt(), 0);
}

void TstClientApi::unknownRouteIs404() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Get("/nope");
    QVERIFY(res);
    QCOMPARE(res->status, 404);
}

static std::string movesPath(const QString& sessionId) {
    return "/sessions/" + sessionId.toStdString() + "/moves";
}

static std::string moveBody(const QString& uci) {
    return R"({"move":")" + uci.toStdString() + R"("})";
}

void TstClientApi::solvingThroughTheApi() {
    httplib::Client cli("127.0.0.1", m_server->port());

    const auto created = cli.Post("/sessions", "", "application/json");
    QVERIFY(created);
    QCOMPARE(created->status, 201);
    const QJsonObject puzzle = jsonBody(created);
    const QString sessionId = puzzle.value("sessionId").toString();
    QVERIFY(!sessionId.isEmpty());

    const Puzzle* p = m_solutions.findById(puzzle.value("puzzleId").toString());
    QVERIFY(p);

    QJsonObject last;
    for (int i = 1; i < p->moves.size(); i += 2) {
        const auto res = cli.Post(movesPath(sessionId), moveBody(p->moves.at(i)), "application/json");
        QVERIFY(res);
        QCOMPARE(res->status, 200);
        last = jsonBody(res);
        QCOMPARE(last.value("verdict").toString(), QStringLiteral("correct"));
    }
    QVERIFY(last.value("solved").toBool());

    const auto stats = cli.Get("/stats");
    QVERIFY(stats);
    QCOMPARE(jsonBody(stats).value("solved").toInt(), 1);

    // the finished session is closed
    const auto again = cli.Post(movesPath(sessionId), moveBody(p->moves.at(1)), "application/json");
    QVERIFY(again);
    QCOMPARE(again->status, 404);
}

void TstClientApi::badBodyIs400() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto created = cli.Post("/sessions", "", "application/json");
    QVERIFY(created);
    const QString sessionId = jsonBody(created).value("sessionId").toString();

    for (const std::string body : {std::string("{}"), std::string("not json")}) {
        const auto res = cli.Post(movesPath(sessionId), body, "application/json");
        QVERIFY(res);
        QCOMPARE(res->status, 400);
        QVERIFY(jsonBody(res).contains("error"));
    }
}

void TstClientApi::unknownSessionIs404() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Post("/sessions/00000000-0000-0000-0000-000000000000/moves",
                              moveBody(QStringLiteral("e2e4")), "application/json");
    QVERIFY(res);
    QCOMPARE(res->status, 404);
}

void TstClientApi::cleanupTestCase() {
    m_server->stop();
    m_server->stop();   // second call must be harmless
    QVERIFY(!m_server->isRunning());
}

QTEST_APPLESS_MAIN(TstClientApi)
#include "tst_clientapi.moc"
