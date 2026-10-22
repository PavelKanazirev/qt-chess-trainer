#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>

#include "TrainerBridge.h"
#include "TrainerService.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("Qt Chess Trainer"));

    QCommandLineParser parser;
    parser.addHelpOption();
    const QCommandLineOption dataOption(
        QStringLiteral("data"), QStringLiteral("Puzzle CSV file."), QStringLiteral("path"),
        QStringLiteral(TRAINER_DATA_DIR "/puzzles_lichess.csv"));
    const QCommandLineOption smokeOption(
        QStringLiteral("smoke"), QStringLiteral("Quit as soon as the UI has loaded."));
    parser.addOption(dataOption);
    parser.addOption(smokeOption);
    parser.process(app);

    TrainerService::Options options;
    options.dataPath = parser.value(dataOption);
    TrainerService service(options);
    QString error;
    if (!service.load(&error)) {
        qCritical("Cannot load puzzles: %s", qPrintable(error));
        return 2;
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