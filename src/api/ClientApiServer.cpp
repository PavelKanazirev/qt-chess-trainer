#include <httplib.h>   // first, before any Qt header

#include "ClientApiServer.h"

#include "ApiJson.h"
#include "Version.h"

using httplib::Request;
using httplib::Response;

namespace {

void reply(Response& res, int status, const QJsonObject& body) {
    res.status = status;
    res.set_content(ApiJson::dump(body), "application/json");
}

}  // namespace

ClientApiServer::ClientApiServer(TrainerService& service, Options options)
    : m_service(service), m_options(std::move(options)) {}

ClientApiServer::~ClientApiServer() { stop(); }

bool ClientApiServer::start(QString* error) {
    if (isRunning()) return true;

    m_server = std::make_unique<httplib::Server>();
    registerRoutes();

    if (m_options.port == 0) {
        m_port = m_server->bind_to_any_port(m_options.host);
    } else {
        m_port = m_server->bind_to_port(m_options.host, m_options.port) ? m_options.port : -1;
    }
    if (m_port < 0) {
        if (error) {
            *error = QStringLiteral("cannot bind %1:%2")
                         .arg(QString::fromStdString(m_options.host))
                         .arg(m_options.port);
        }
        m_server.reset();
        return false;
    }

    m_thread = std::thread([this] { m_server->listen_after_bind(); });
    return true;
}

void ClientApiServer::stop() {
    if (m_server) m_server->stop();
    if (m_thread.joinable()) m_thread.join();
}

void ClientApiServer::registerRoutes() {
    m_server->Get("/health", [this](const Request&, Response& res) {
        reply(res, 200, {
            {"status", QStringLiteral("ok")},
            {"puzzles", m_service.puzzleCount()},
            {"version", trainerVersion()},
        });
    });

    m_server->Get("/stats", [this](const Request&, Response& res) {
        reply(res, 200, ApiJson::stats(m_service.stats()));
    });

    m_server->Post("/sessions", [this](const Request&, Response& res) {
        const auto view = m_service.startNext();
        if (!view) {
            reply(res, 503, ApiJson::error(QStringLiteral("no puzzle available")));
            return;
        }
        reply(res, 201, ApiJson::puzzle(*view));
    });

    // Session ids are UUIDs; anything else does not match and gets a 404 from the router.
    m_server->Post(R"(/sessions/([0-9a-fA-F-]+)/moves)", [this](const Request& req, Response& res) {
        const QString sessionId = QString::fromStdString(req.matches[1].str());
        const auto move = ApiJson::moveFromBody(req.body);
        if (!move) {
            reply(res, 400, ApiJson::error(QStringLiteral(R"(body must be {"move": "<uci>"})")));
            return;
        }
        const MoveOutcome out = m_service.submitMove(sessionId, *move);
        if (!out.sessionFound) {
            reply(res, 404, ApiJson::error(QStringLiteral("unknown or finished session")));
            return;
        }
        reply(res, 200, ApiJson::moveOutcome(out));   // illegal/wrong are verdicts, not HTTP errors
    });
}
