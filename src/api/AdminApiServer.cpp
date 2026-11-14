#include <httplib.h>   // first, before any Qt header

#include "AdminApiServer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <optional>

#include "ApiJson.h"

using httplib::Request;
using httplib::Response;

namespace {

// Compares without stopping at the first difference, so timing does not reveal the token.
bool equalConstantTime(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

std::optional<QJsonObject> objectFromBody(const std::string& body) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(body), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) return std::nullopt;
    return doc.object();
}

}  // namespace

AdminApiServer::AdminApiServer(ConfigService& config, Options options)
    : ApiServerBase(std::move(options)), m_config(config) {}

AdminApiServer::~AdminApiServer() { stop(); }

QJsonObject AdminApiServer::configBody(const ConfigResult* result) const {
    const QStringList restart = result ? result->restartRequired : m_config.restartPending();
    return {
        {"config", ConfigService::toJson(m_config.current(), true)},
        {"defaults", ConfigService::toJson(TrainerConfig{}, true)},
        {"selectors", QJsonArray::fromStringList(m_config.selectors())},
        {"restartRequired", QJsonArray::fromStringList(restart)},
    };
}

void AdminApiServer::registerRoutes(httplib::Server& server) {
    // Runs before routing, so every path (even an unknown one) needs the token.
    server.set_pre_routing_handler([this](const Request& req, Response& res) {
        const std::string expected = m_config.current().adminToken.toStdString();
        if (!equalConstantTime(req.get_header_value("X-Admin-Token"), expected)) {
            reply(res, 401, ApiJson::error(QStringLiteral("missing or wrong X-Admin-Token")));
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    server.Get("/config", [this](const Request&, Response& res) {
        reply(res, 200, configBody());
    });

    server.Put("/config", [this](const Request& req, Response& res) {
        const auto patch = objectFromBody(req.body);
        if (!patch) {
            reply(res, 400, ApiJson::error(QStringLiteral("body must be a JSON object")));
            return;
        }
        const ConfigResult r = m_config.apply(*patch);
        if (!r.ok) {
            reply(res, 400, {{"errors", QJsonArray::fromStringList(r.errors)}});
            return;
        }
        reply(res, 200, configBody(&r));
    });

    server.Post("/config/reset", [this](const Request& req, Response& res) {
        QStringList keys;   // empty: reset everything
        if (!req.body.empty()) {
            const auto body = objectFromBody(req.body);
            const QJsonValue list = body ? body->value(QStringLiteral("keys")) : QJsonValue();
            if (!body || (!list.isUndefined() && !list.isArray())) {
                reply(res, 400, ApiJson::error(QStringLiteral(R"(body must be empty or {"keys": [...]})")));
                return;
            }
            for (const QJsonValue& key : list.toArray()) keys << key.toString();
        }
        const ConfigResult r = m_config.reset(keys);
        if (!r.ok) {
            reply(res, 400, {{"errors", QJsonArray::fromStringList(r.errors)}});
            return;
        }
        reply(res, 200, configBody(&r));
    });
}
