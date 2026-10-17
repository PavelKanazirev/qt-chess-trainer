#include "MoveValidator.h"

namespace MoveValidator {

MoveVerdict check(const ChessPosition& position, const QString& played,
                  const QString& expected) {
    const QString move = played.trimmed().toLower();
    if (!position.isLegal(move)) return MoveVerdict::Illegal;
    if (move == expected.trimmed().toLower()) return MoveVerdict::Correct;

    ChessPosition after = position;
    after.play(move);
    return after.isCheckmate() ? MoveVerdict::Correct : MoveVerdict::Wrong;
}

}  // namespace MoveValidator
