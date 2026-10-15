#include "PuzzleParser.h"

#include <QRegularExpression>

namespace {
bool isUciMove(const QString& m) {
    static const QRegularExpression re(QStringLiteral("^[a-h][1-8][a-h][1-8][qrbn]?$"));
    return re.match(m).hasMatch();
}
} // namespace

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

    if (p.id.isEmpty() || p.rating <= 0) return std::nullopt;
    if (p.fen.split(QLatin1Char(' ')).size() != 6) return std::nullopt;
    if (p.moves.size() < 2) return std::nullopt;
    for (const QString& m : std::as_const(p.moves))
        if (!isUciMove(m)) return std::nullopt;
    return p;
}

}  // namespace PuzzleParser
