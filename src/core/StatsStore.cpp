#include "StatsStore.h"

#include <cmath>

namespace {
constexpr double kK = 32.0;   // Elo K-factor
}

StatsStore::StatsStore(int initialRating, int recentLimit)
    : m_initialRating(initialRating), m_recentLimit(recentLimit), m_rating(initialRating) {}

void StatsStore::record(const Puzzle& puzzle, bool solved) {
    const double expected = 1.0 / (1.0 + std::pow(10.0, (puzzle.rating - m_rating) / 400.0));
    const double score = solved ? 1.0 : 0.0;
    m_rating += static_cast<int>(std::lround(kK * (score - expected)));

    if (solved) ++m_solved; else ++m_failed;
    for (const QString& theme : puzzle.themes) {
        ThemeStats& t = m_themes[theme];
        if (solved) ++t.solved; else ++t.failed;
    }

    m_recent.removeAll(puzzle.id);
    m_recent.push_back(puzzle.id);
    while (m_recent.size() > m_recentLimit) m_recent.removeFirst();
}

void StatsStore::reset() {
    m_rating = m_initialRating;
    m_solved = 0;
    m_failed = 0;
    m_themes.clear();
    m_recent.clear();
}

SelectionContext StatsStore::selectionContext(int ratingWindow) const {
    SelectionContext ctx;
    ctx.playerRating = m_rating;
    ctx.ratingWindow = ratingWindow;
    for (auto it = m_themes.cbegin(); it != m_themes.cend(); ++it)
        if (it->failed > 0) ctx.themeFailures.insert(it.key(), it->failed);
    for (const QString& id : m_recent) ctx.recentIds.insert(id);
    return ctx;
}
