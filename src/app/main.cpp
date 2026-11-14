#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>

#include "AdminApiServer.h"
#include "ClientApiServer.h"
#include "ConfigService.h"
#include "SelectorRegistry.h"
#include "TrainerBridge.h"
#include "TrainerService.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Qt Chess Trainer"));

    QCommandLineParser parser;
    parser.addHelpOption();
    const QCommandLineOption configOption(
        QStringLiteral("config"), QStringLiteral("Configuration file (created on first change)."),
        QStringLiteral("path"), QStringLiteral(TRAINER_CONFIG_DIR "/config.local.json"));
    const QCommandLineOption dataOption(
        QStringLiteral("data"), QStringLiteral("Puzzle CSV file (overrides the config)."),
        QStringLiteral("path"));
    const QCommandLineOption apiPortOption(
        QStringLiteral("api-port"), QStringLiteral("Client REST API port (overrides the config)."),
        QStringLiteral("port"));
    const QCommandLineOption adminPortOption(
        QStringLiteral("admin-port"), QStringLiteral("Admin REST API port (overrides the config)."),
        QStringLiteral("port"));
    const QCommandLineOption noApiOption(
        QStringLiteral("no-api"), QStringLiteral("Start neither REST API."));
    const QCommandLineOption smokeOption(
        QStringLiteral("smoke"), QStringLiteral("Quit as soon as the UI has loaded."));
    parser.addOptions({configOption, dataOption, apiPortOption, adminPortOption,
                       noApiOption, smokeOption});
    parser.process(app);

    ConfigService config(parser.value(configOption), SelectorRegistry::withDefaults().names());
    QString error;
    if (!config.load(&error)) {
        qCritical("Cannot load configuration: %s", qPrintable(error));
        return 2;
    }
    const TrainerConfig cfg = config.current();

    TrainerService::Options options;
    options.selectorName = cfg.selector;
    options.ratingWindow = cfg.ratingWindow;
    if (parser.isSet(dataOption))
        options.dataPath = parser.value(dataOption);
    else if (!cfg.dataPath.isEmpty())   // relative paths are relative to the config file
        options.dataPath = QFileInfo(QFileInfo(config.path()).dir(), cfg.dataPath).absoluteFilePath();
    else
        options.dataPath = QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv");

    TrainerService service(options);
    if (!service.load(&error)) {
        qCritical("Cannot load puzzles: %s", qPrintable(error));
        return 2;
    }

    // Observer: configuration changes from the admin API reach the facade at once.
    config.addListener([&service](const TrainerConfig& c) {
        service.setSelector(c.selector);
        service.setRatingWindow(c.ratingWindow);
    });

    auto portFrom = [&parser](const QCommandLineOption& option, int fallback) {
        return parser.isSet(option) ? parser.value(option).toInt() : fallback;
    };
    ClientApiServer::Options clientOptions;
    clientOptions.port = portFrom(apiPortOption, cfg.clientApiPort);
    AdminApiServer::Options adminOptions;
    adminOptions.port = portFrom(adminPortOption, cfg.adminApiPort);

    // Declared after 'service' and 'config', so they stop before those are destroyed.
    ClientApiServer clientApi(service, clientOptions);
    AdminApiServer adminApi(config, adminOptions);
    if (!parser.isSet(noApiOption)) {
        if (clientApi.start(&error))
            qInfo("Client REST API on http://127.0.0.1:%d", clientApi.port());
        else
            qWarning("Client REST API not started: %s", qPrintable(error));

        if (cfg.adminToken == TrainerConfig{}.adminToken)
            qWarning("Admin API uses the default token; change it in %s", qPrintable(config.path()));
        if (adminApi.start(&error))
            qInfo("Admin REST API on http://127.0.0.1:%d", adminApi.port());
        else
            qWarning("Admin REST API not started: %s", qPrintable(error));
    }

    TrainerBridge bridge(service);
    bridge.nextPuzzle();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({{QStringLiteral("trainer"), QVariant::fromValue(&bridge)}});

    const bool smoke = parser.isSet(smokeOption);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
        [&](QObject* obj, const QUrl&) {
            if (!obj) {
                QCoreApplication::exit(1);
            } else if (smoke) {
                QTimer::singleShot(0, &app, &QCoreApplication::quit);
            }
        }, Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/ChessTrainer/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) return 1;
    return app.exec();
}