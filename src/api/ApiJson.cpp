#include "ApiJson.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

namespace ApiJson {

QString verdictName(MoveVerdict verdict) {
    switch (verdict) {
    case MoveVerdict::Correct: return QStringLiteral("correct");
    case MoveVerdict::Wrong: return QStringLiteral("wrong");
    case MoveVerdict::Illegal: return QStringLiteral("illegal");
    }
    return QStringLiteral("unknown");
}

QJsonObject puzzle(const PuzzleView& view) {
    return {
        {"sessionId", view.sessionId},
        {"puzzleId", view.puzzleId},
        {"fen", view.fen},
        {"lastMove", view.lastMove},
        {"rating", view.rating},
        {"themes", QJsonArray::fromStringList(view.themes)},
        {"sideToMove", view.whiteToMove ? QStringLiteral("white") : QStringLiteral("black")},
    };
}

QJsonObject moveOutcome(const MoveOutcome& outcome) {
    return {
        {"verdict", verdictName(outcome.verdict)},
        {"opponentReply", outcome.opponentReply},
        {"solved", outcome.solved},
        {"failed", outcome.failed},
        {"fen", outcome.fen},
        {"fenAfterMove", outcome.fenAfterMove},
        {"playerRating", outcome.playerRating},
    };
}

QJsonObject stats(const StatsSnapshot& stats) {
    QJsonObject themes;
    for (auto it = stats.themes.cbegin(); it != stats.themes.cend(); ++it)
        themes.insert(it.key(), QJsonObject{{"solved", it->solved}, {"failed", it->failed}});
    return {
        {"playerRating", stats.playerRating},
        {"solved", stats.solved},
        {"failed", stats.failed},
        {"selector", stats.selector},
        {"themes", themes},
    };
}

QJsonObject error(const QString& message) {
    return {{"error", message}};
}

std::string dump(const QJsonObject& object) {
    return QJsonDocument(object).toJson(QJsonDocument::Compact).toStdString();
}

std::optional<QString> moveFromBody(const std::string& body) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(body), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) return std::nullopt;

    const QJsonValue value = doc.object().value(QStringLiteral("move"));
    if (!value.isString()) return std::nullopt;
    const QString move = value.toString().trimmed();
    if (move.size() < 4 || move.size() > 5) return std::nullopt;   // UCI: e2e4 or e7e8q
    return move;
}

}  // namespace ApiJson
