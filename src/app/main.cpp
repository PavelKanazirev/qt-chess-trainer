#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>
#include <QUrl>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    const bool smoke = app.arguments().contains(QStringLiteral("--smoke"));

    QQmlApplicationEngine engine;
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
