#include <httplib.h>   // first, before any Qt header

#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <memory>

#include "AdminApiServer.h"
#include "ConfigService.h"
#include "TrainerService.h"

class TstAdminApi : public QObject {
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void requestsWithoutTokenAre401();
    void getConfigMasksToken();
    void putSelectorReachesTrainerService();
    void invalidPatchIs400();
    void portChangeNeedsRestart();
    void resetRestoresDefaults();
    void newTokenTakesEffect();

private:
    std::unique_ptr<QTemporaryDir> m_dir;
    std::unique_ptr<TrainerService> m_service;
    std::unique_ptr<ConfigService> m_config;
    std::unique_ptr<AdminApiServer> m_server;
};

static const char* kToken = "test-token-123";
static const char* kJson = "application/json";

static httplib::Headers auth(const char* token = kToken) {
    return {{"X-Admin-Token", token}};
}

static QJsonObject json(const httplib::Result& res) {
    return QJsonDocument::fromJson(QByteArray::fromStdString(res->body)).object();
}

void TstAdminApi::init() {
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());
    const QString path = m_dir->filePath(QStringLiteral("config.json"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(QByteArray(R"({"adminToken": ")") + kToken + R"("})");
    f.close();

    TrainerService::Options o;
    o.dataPath = QStringLiteral(TRAINER_DATA_DIR "/puzzles_sample.csv");
    o.seed = 1;
    m_service = std::make_unique<TrainerService>(o);
    QVERIFY(m_service->load());

    m_config = std::make_unique<ConfigService>(path, m_service->availableSelectors());
    QVERIFY(m_config->load());
    m_config->addListener([svc = m_service.get()](const TrainerConfig& c) {
        svc->setSelector(c.selector);
        svc->setRatingWindow(c.ratingWindow);
    });

    AdminApiServer::Options so;
    so.port = 0;
    m_server = std::make_unique<AdminApiServer>(*m_config, so);
    QString error;
    QVERIFY2(m_server->start(&error), qPrintable(error));
}

void TstAdminApi::cleanup() {
    m_server.reset();   // stop the server before what its handlers use
    m_config.reset();
    m_service.reset();
    m_dir.reset();
}

void TstAdminApi::requestsWithoutTokenAre401() {
    httplib::Client cli("127.0.0.1", m_server->port());
    auto res = cli.Get("/config");
    QVERIFY(res);
    QCOMPARE(res->status, 401);

    res = cli.Get("/config", auth("wrong-token-000"));
    QCOMPARE(res->status, 401);

    res = cli.Put("/config", auth("wrong-token-000"), R"({"selector":"Random"})", kJson);
    QCOMPARE(res->status, 401);
    QCOMPARE(m_service->selectorName(), QStringLiteral("ByRating"));

    res = cli.Get("/nope");   // even unknown routes need the token
    QCOMPARE(res->status, 401);
}

void TstAdminApi::getConfigMasksToken() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Get("/config", auth());
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    const QJsonObject body = json(res);
    QCOMPARE(body.value("config").toObject().value("adminToken").toString(), QStringLiteral("***"));
    QCOMPARE(body.value("defaults").toObject().value("selector").toString(), QStringLiteral("ByRating"));
    QCOMPARE(body.value("selectors").toArray().size(), 3);
}

void TstAdminApi::putSelectorReachesTrainerService() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Put("/config", auth(), R"({"selector":"Random","ratingWindow":300})", kJson);
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    QCOMPARE(m_service->selectorName(), QStringLiteral("Random"));
    QCOMPARE(m_service->ratingWindow(), 300);

    QFile f(m_config->path());
    QVERIFY(f.open(QIODevice::ReadOnly));
    QVERIFY(f.readAll().contains("\"Random\""));
}

void TstAdminApi::invalidPatchIs400() {
    httplib::Client cli("127.0.0.1", m_server->port());
    auto res = cli.Put("/config", auth(), R"({"ratingWindow":5,"nope":1})", kJson);
    QVERIFY(res);
    QCOMPARE(res->status, 400);
    QCOMPARE(json(res).value("errors").toArray().size(), 2);
    QCOMPARE(m_service->ratingWindow(), 200);

    res = cli.Put("/config", auth(), "not json", kJson);
    QCOMPARE(res->status, 400);
}

void TstAdminApi::portChangeNeedsRestart() {
    httplib::Client cli("127.0.0.1", m_server->port());
    const auto res = cli.Put("/config", auth(), R"({"clientApiPort":9090})", kJson);
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    QVERIFY(json(res).value("restartRequired").toArray().contains(QStringLiteral("clientApiPort")));

    const auto get = cli.Get("/config", auth());
    QVERIFY(json(get).value("restartRequired").toArray().contains(QStringLiteral("clientApiPort")));
}

void TstAdminApi::resetRestoresDefaults() {
    httplib::Client cli("127.0.0.1", m_server->port());
    QCOMPARE(cli.Put("/config", auth(), R"({"selector":"Random"})", kJson)->status, 200);

    auto res = cli.Post("/config/reset", auth(), R"({"keys":["selector"]})", kJson);
    QVERIFY(res);
    QCOMPARE(res->status, 200);
    QCOMPARE(m_service->selectorName(), QStringLiteral("ByRating"));

    res = cli.Post("/config/reset", auth(), R"({"keys":["nope"]})", kJson);
    QCOMPARE(res->status, 400);
}

void TstAdminApi::newTokenTakesEffect() {
    httplib::Client cli("127.0.0.1", m_server->port());
    QCOMPARE(cli.Put("/config", auth(), R"({"adminToken":"another-token-456"})", kJson)->status, 200);
    QCOMPARE(cli.Get("/config", auth())->status, 401);
    QCOMPARE(cli.Get("/config", auth("another-token-456"))->status, 200);
}

QTEST_APPLESS_MAIN(TstAdminApi)
#include "tst_adminapi.moc"
