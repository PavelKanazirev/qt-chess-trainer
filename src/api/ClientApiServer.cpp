#include <httplib.h>   // first, before any Qt header

#include "ClientApiServer.h"

#include "ApiJson.h"
#include "Version.h"

using httplib::Request;
using httplib::Response;

ClientApiServer::ClientApiServer(TrainerService& service, Options options)
    : ApiServerBase(std::move(options)), m_service(service) {}

ClientApiServer::~ClientApiServer() { stop(); }

void ClientApiServer::registerRoutes(httplib::Server& server) {
    server.Get("/health", [this](const Request&, Response& res) {
        reply(res, 200, {
            {"status", QStringLiteral("ok")},
            {"puzzles", m_service.puzzleCount()},
            {"version", trainerVersion()},
        });
    });

    server.Get("/stats", [this](const Request&, Response& res) {
        reply(res, 200, ApiJson::stats(m_service.stats()));
    });

    server.Post("/sessions", [this](const Request&, Response& res) {
        const auto view = m_service.startNext();
        if (!view) {
            reply(res, 503, ApiJson::error(QStringLiteral("no puzzle available")));
            return;
        }
        reply(res, 201, ApiJson::puzzle(*view));
    });

    // Session ids are UUIDs; anything else does not match and gets a 404 from the router.
    server.Post(R"(/sessions/([0-9a-fA-F-]+)/moves)", [this](const Request& req, Response& res) {
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
