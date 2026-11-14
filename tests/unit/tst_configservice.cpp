#include <QtTest>
#include <QFile>
#include <QTemporaryDir>
#include <memory>
#include "ConfigService.h"

class TstConfigService : public QObject {
    Q_OBJECT
private slots:
    void init();
    void missingFileGivesDefaults();
    void defaultFileMatchesBuiltinDefaults();
    void partialFileKeepsOtherDefaults();
    void invalidFileFailsToLoad();
    void validPatchIsPersisted();
    void invalidPatchChangesNothing();
    void portsMustDiffer();
    void portChangeNeedsRestart();
    void resetRestoresDefaults();
    void listenersSeeNewConfig();

private:
    QString path() const { return m_dir->filePath(QStringLiteral("config.json")); }
    void writeFile(const QByteArray& content) const;
    std::unique_ptr<QTemporaryDir> m_dir;
};

static const QStringList kSelectors{QStringLiteral("ByRating"), QStringLiteral("Random"),
                                    QStringLiteral("WeakestTheme")};

void TstConfigService::init() {
    m_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_dir->isValid());
}

void TstConfigService::writeFile(const QByteArray& content) const {
    QFile f(path());
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(content);
}

void TstConfigService::missingFileGivesDefaults() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    QVERIFY(c.current() == TrainerConfig{});
    QVERIFY(!QFile::exists(path()));   // nothing written until something changes
}

void TstConfigService::defaultFileMatchesBuiltinDefaults() {
    ConfigService c(QStringLiteral(TRAINER_CONFIG_DIR "/config.default.json"), kSelectors);
    QString error;
    QVERIFY2(c.load(&error), qPrintable(error));
    QVERIFY(c.current() == TrainerConfig{});
}

void TstConfigService::partialFileKeepsOtherDefaults() {
    writeFile(R"({"ratingWindow": 300})");
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    QCOMPARE(c.current().ratingWindow, 300);
    QCOMPARE(c.current().selector, QStringLiteral("ByRating"));
}

void TstConfigService::invalidFileFailsToLoad() {
    writeFile(R"({"ratingWindow": "wide"})");
    ConfigService c(path(), kSelectors);
    QString error;
    QVERIFY(!c.load(&error));
    QVERIFY(error.contains(QStringLiteral("ratingWindow")));

    writeFile("not json");
    QVERIFY(!c.load(&error));
}

void TstConfigService::validPatchIsPersisted() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    const ConfigResult r = c.apply({{"selector", "Random"}, {"ratingWindow", 150}});
    QVERIFY(r.ok);
    QVERIFY(r.errors.isEmpty());

    ConfigService again(path(), kSelectors);
    QVERIFY(again.load());
    QCOMPARE(again.current().selector, QStringLiteral("Random"));
    QCOMPARE(again.current().ratingWindow, 150);
}

void TstConfigService::invalidPatchChangesNothing() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    const ConfigResult r = c.apply({{"selector", "Nope"}, {"ratingWindow", 5}, {"colour", "blue"}});
    QVERIFY(!r.ok);
    QCOMPARE(r.errors.size(), 3);
    QVERIFY(c.current() == TrainerConfig{});
    QVERIFY(!QFile::exists(path()));
}

void TstConfigService::portsMustDiffer() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    const ConfigResult r = c.apply({{"clientApiPort", 8081}});
    QVERIFY(!r.ok);
    QVERIFY(r.errors.join(QStringLiteral(" ")).contains(QStringLiteral("differ")));
}

void TstConfigService::portChangeNeedsRestart() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    ConfigResult r = c.apply({{"clientApiPort", 9090}});
    QVERIFY(r.ok);
    QCOMPARE(r.restartRequired, QStringList{QStringLiteral("clientApiPort")});
    QCOMPARE(c.restartPending(), QStringList{QStringLiteral("clientApiPort")});

    r = c.apply({{"clientApiPort", 8080}});   // back to what the server runs with
    QVERIFY(r.restartRequired.isEmpty());

    r = c.apply({{"selector", "Random"}});    // applied live, no restart
    QVERIFY(r.restartRequired.isEmpty());
}

void TstConfigService::resetRestoresDefaults() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    QVERIFY(c.apply({{"selector", "Random"}, {"ratingWindow", 300}}).ok);

    QVERIFY(c.reset({QStringLiteral("selector")}).ok);
    QCOMPARE(c.current().selector, QStringLiteral("ByRating"));
    QCOMPARE(c.current().ratingWindow, 300);

    QVERIFY(c.reset().ok);
    QVERIFY(c.current() == TrainerConfig{});

    QVERIFY(!c.reset({QStringLiteral("nope")}).ok);
}

void TstConfigService::listenersSeeNewConfig() {
    ConfigService c(path(), kSelectors);
    QVERIFY(c.load());
    QList<TrainerConfig> seen;
    c.addListener([&seen](const TrainerConfig& cfg) { seen.push_back(cfg); });

    QVERIFY(c.apply({{"selector", "WeakestTheme"}}).ok);
    QVERIFY(!c.apply({{"ratingWindow", 5}}).ok);   // invalid: no notification

    QCOMPARE(seen.size(), 1);
    QCOMPARE(seen.first().selector, QStringLiteral("WeakestTheme"));
}

QTEST_APPLESS_MAIN(TstConfigService)
#include "tst_configservice.moc"
