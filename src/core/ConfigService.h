#pragma once
#include <functional>
#include <mutex>
#include <vector>
#include <QJsonObject>
#include <QString>
#include <QStringList>

struct TrainerConfig {
    QString selector = QStringLiteral("ByRating");
    int ratingWindow = 200;
    int clientApiPort = 8080;
    int adminApiPort = 8081;
    QString adminToken = QStringLiteral("change-me");
    QString dataPath;   // empty: the built-in Lichess sample

    bool operator==(const TrainerConfig&) const = default;
};

struct ConfigResult {
    bool ok = false;
    QStringList errors;
    QStringList restartRequired;   // changed keys that take effect after a restart
};

// Owns config.json: validates, persists, and notifies listeners (Observer). Thread-safe.
class ConfigService {
public:
    using Listener = std::function<void(const TrainerConfig&)>;

    ConfigService(QString path, QStringList selectors);

    static QStringList keys();
    static QStringList restartKeys();
    static QJsonObject toJson(const TrainerConfig& config, bool maskToken = false);

    bool load(QString* error = nullptr);   // a missing file means defaults
    TrainerConfig current() const;
    QString path() const { return m_path; }
    QStringList selectors() const { return m_selectors; }

    ConfigResult apply(const QJsonObject& patch);
    ConfigResult reset(const QStringList& keys = {});   // empty: every key
    QStringList restartPending() const;

    // Called after a successful apply/reset, outside the internal lock.
    void addListener(Listener listener);

private:
    QStringList validate(const QJsonObject& patch, TrainerConfig& config) const;
    QStringList restartPendingLocked() const;
    bool save(const TrainerConfig& config, QString* error) const;

    QString m_path;
    QStringList m_selectors;
    mutable std::mutex m_mutex;
    TrainerConfig m_config;
    TrainerConfig m_startup;   // what the running servers were started with
    std::vector<Listener> m_listeners;
};
