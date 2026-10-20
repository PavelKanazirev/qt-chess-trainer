#pragma once
#include <QHash>
#include <QString>
#include <QStringList>
#include "MoveValidator.h"
#include "StatsStore.h"

// Value types returned by TrainerService. Callers get copies, never references.

struct PuzzleView {
    QString sessionId;
    QString puzzleId;
    QString fen;          // position after the opponent's first move
    QString lastMove;     // that opponent move, for highlighting
    int rating = 0;
    QStringList themes;
    bool whiteToMove = true;
};

struct MoveOutcome {
    bool sessionFound = false;
    MoveVerdict verdict = MoveVerdict::Illegal;
    QString opponentReply;
    bool solved = false;
    bool failed = false;
    QString fen;
    int playerRating = 0;
};

struct StatsSnapshot {
    int playerRating = 0;
    int solved = 0;
    int failed = 0;
    QHash<QString, ThemeStats> themes;
    QString selector;
};
