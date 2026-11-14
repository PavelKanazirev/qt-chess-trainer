#pragma once
#include <optional>
#include <string>
#include <QJsonObject>
#include <QString>
#include "TrainerTypes.h"

// Conversions between the facade's value types and the JSON used by the REST API.
namespace ApiJson {

QString verdictName(MoveVerdict verdict);              // "correct", "wrong", "illegal"
QJsonObject puzzle(const PuzzleView& view);
QJsonObject moveOutcome(const MoveOutcome& outcome);
QJsonObject stats(const StatsSnapshot& stats);
QJsonObject error(const QString& message);

std::string dump(const QJsonObject& object);           // compact UTF-8 JSON
std::optional<QString> moveFromBody(const std::string& body);   // {"move": "e2e4"}

}  // namespace ApiJson
