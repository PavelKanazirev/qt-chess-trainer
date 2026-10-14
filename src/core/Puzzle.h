#pragma once
#include <QString>
#include <QStringList>

struct Puzzle {
    QString id;
    QString fen;        // position before the opponent's first move
    QStringList moves;  // UCI; moves[0] is the opponent's move, auto-played
    int rating = 0;
    QStringList themes;
};
