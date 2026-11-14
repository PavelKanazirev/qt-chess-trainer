#include "ConfigService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <cmath>

ConfigService::ConfigService(QString path, QStringList selectors)
    : m_path(std::move(path)), m_selectors(std::move(selectors)) {}

QStringList ConfigService::keys() {
    return {QStringLiteral("selector"), QStringLiteral("ratingWindow"),
            QStringLiteral("clientApiPort"), QStringLiteral("adminApiPort"),
            QStringLiteral("adminToken"), QStringLiteral("dataPath")};
}

QStringList ConfigService::restartKeys() {
    return {QStringLiteral("clientApiPort"), QStringLiteral("adminApiPort"),
            QStringLiteral("dataPath")};
}

QJsonObject ConfigService::toJson(const TrainerConfig& c, bool maskToken) {
    return {
        {"selector", c.selector},
        {"ratingWindow", c.ratingWindow},
        {"clientApiPort", c.clientApiPort},
        {"adminApiPort", c.adminApiPort},
        {"adminToken", maskToken ? QStringLiteral("***") : c.adminToken},
        {"dataPath", c.dataPath},
    };
}

bool ConfigService::load(QString* error) {
    std::lock_guard lock(m_mutex);
    TrainerConfig config;   // defaults

    QFile file(m_path);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            if (error) *error = QStringLiteral("%1: %2").arg(m_path, file.errorString());
            return false;
        }
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            if (error) *error = QStringLiteral("%1: not a JSON object").arg(m_path);
            return false;
        }
        const QStringList errors = validate(doc.object(), config);
        if (!errors.isEmpty()) {
            if (error) *error = QStringLiteral("%1: %2").arg(m_path, errors.join(QStringLiteral("; ")));
            return false;
        }
    }
    m_config = config;
    m_startup = config;
    return true;
}

TrainerConfig ConfigService::current() const {
    std::lock_guard lock(m_mutex);
    return m_config;
}

ConfigResult ConfigService::apply(const QJsonObject& patch) {
    ConfigResult result;
    TrainerConfig next;
    std::vector<Listener> listeners;
    {
        std::lock_guard lock(m_mutex);
        next = m_config;
        result.errors = validate(patch, next);
        if (!result.errors.isEmpty()) return result;

        QString error;
        if (!save(next, &error)) {
            result.errors << error;
            return result;
        }
        m_config = next;
        result.ok = true;
        result.restartRequired = restartPendingLocked();
        listeners = m_listeners;
    }
    for (const Listener& listener : listeners) listener(next);   // outside the lock
    return result;
}

ConfigResult ConfigService::reset(const QStringList& keys) {
    const QStringList chosen = keys.isEmpty() ? ConfigService::keys() : keys;
    ConfigResult result;
    for (const QString& key : chosen)
        if (!ConfigService::keys().contains(key)) result.errors << QStringLiteral("unknown key: %1").arg(key);
    if (!result.errors.isEmpty()) return result;

    const QJsonObject defaults = toJson(TrainerConfig{});
    QJsonObject patch;
    for (const QString& key : chosen) patch.insert(key, defaults.value(key));
    return apply(patch);
}

QStringList ConfigService::restartPending() const {
    std::lock_guard lock(m_mutex);
    return restartPendingLocked();
}

void ConfigService::addListener(Listener listener) {
    std::lock_guard lock(m_mutex);
    m_listeners.push_back(std::move(listener));
}

QStringList ConfigService::restartPendingLocked() const {
    const QJsonObject now = toJson(m_config);
    const QJsonObject startup = toJson(m_startup);
    QStringList changed;
    for (const QString& key : restartKeys())
        if (now.value(key) != startup.value(key)) changed << key;
    return changed;
}

QStringList ConfigService::validate(const QJsonObject& patch, TrainerConfig& out) const {
    QStringList errors;
    auto readInt = [&errors](const QString& key, const QJsonValue& v, int min, int max, int& target) {
        const double d = v.toDouble();
        if (!v.isDouble() || d != std::floor(d) || d < min || d > max) {
            errors << QStringLiteral("%1 must be an integer between %2 and %3").arg(key).arg(min).arg(max);
            return;
        }
        target = static_cast<int>(d);
    };

    for (auto it = patch.begin(); it != patch.end(); ++it) {
        const QString key = it.key();
        const QJsonValue v = it.value();
        if (key == QLatin1String("selector")) {
            if (!v.isString() || !m_selectors.contains(v.toString()))
                errors << QStringLiteral("selector must be one of: %1").arg(m_selectors.join(QStringLiteral(", ")));
            else
                out.selector = v.toString();
        } else if (key == QLatin1String("ratingWindow")) {
            readInt(key, v, 50, 1000, out.ratingWindow);
        } else if (key == QLatin1String("clientApiPort")) {
            readInt(key, v, 1024, 65535, out.clientApiPort);
        } else if (key == QLatin1String("adminApiPort")) {
            readInt(key, v, 1024, 65535, out.adminApiPort);
        } else if (key == QLatin1String("adminToken")) {
            if (!v.isString() || v.toString().size() < 8)
                errors << QStringLiteral("adminToken must be a string of at least 8 characters");
            else
                out.adminToken = v.toString();
        } else if (key == QLatin1String("dataPath")) {
            if (!v.isString()) errors << QStringLiteral("dataPath must be a string");
            else out.dataPath = v.toString();
        } else {
            errors << QStringLiteral("unknown key: %1").arg(key);
        }
    }
    if (errors.isEmpty() && out.clientApiPort == out.adminApiPort)
        errors << QStringLiteral("clientApiPort and adminApiPort must differ");
    return errors;
}

bool ConfigService::save(const TrainerConfig& config, QString* error) const {
    QDir().mkpath(QFileInfo(m_path).absolutePath());
    QSaveFile file(m_path);   // writes a temporary file, renames on commit: never half-written
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = QStringLiteral("%1: %2").arg(m_path, file.errorString());
        return false;
    }
    file.write(QJsonDocument(toJson(config)).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error) *error = QStringLiteral("%1: %2").arg(m_path, file.errorString());
        return false;
    }
    return true;
}
