#pragma once
#include <QString>
#include "ChessPosition.h"

enum class MoveVerdict { Correct, Wrong, Illegal };

namespace MoveValidator {
// 'expected' is the solution move for this ply. A different legal move that gives
// checkmate is also accepted (as on Lichess).
MoveVerdict check(const ChessPosition& position, const QString& played,
                  const QString& expected);
}
