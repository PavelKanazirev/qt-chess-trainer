#pragma once
#include <QHash>
#include <QList>
#include <QString>
#include "IPuzzleSelector.h"
#include "Puzzle.h"

struct ThemeStats {
    int solved = 0;
    int failed = 0;
};

// Player statistics and rating estimate. Not thread-safe: TrainerService guards it.
class StatsStore {
public:
    explicit StatsStore(int initialRating = 1500, int recentLimit = 20);

    void record(const Puzzle& puzzle, bool solved);
    void reset();

    int playerRating() const { return m_rating; }
    int solvedCount() const { return m_solved; }
    int failedCount() const { return m_failed; }
    const QHash<QString, ThemeStats>& themes() const { return m_themes; }
    const QList<QString>& recentIds() const { return m_recent; }

    SelectionContext selectionContext(int ratingWindow) const;

private:
    int m_initialRating;
    int m_recentLimit;
    int m_rating;
    int m_solved = 0;
    int m_failed = 0;
    QHash<QString, ThemeStats> m_themes;
    QList<QString> m_recent;
};
