#pragma once
#include <optional>
#include <QString>
#include "Puzzle.h"

namespace PuzzleParser {
// Parses one CSV line. Returns nullopt if the line is not a valid puzzle.
std::optional<Puzzle> parseLine(const QString& line);
}
