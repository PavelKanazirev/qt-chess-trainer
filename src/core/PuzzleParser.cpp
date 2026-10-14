#include "PuzzleParser.h"

namespace PuzzleParser {

std::optional<Puzzle> parseLine(const QString& line) {
    const QStringList f = line.trimmed().split(QLatin1Char(','), Qt::KeepEmptyParts);
    if (f.size() < 8) return std::nullopt;

    Puzzle p;
    p.id = f[0];
    p.fen = f[1];
    p.moves = f[2].split(QLatin1Char(' '), Qt::SkipEmptyParts);
    bool ok = false;
    p.rating = f[3].toInt(&ok);
    if (!ok) return std::nullopt;
    p.themes = f[7].split(QLatin1Char(' '), Qt::SkipEmptyParts);
    return p;
}

}  // namespace PuzzleParser
