#pragma once
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
#include <algorithm>
#include <cstdlib>
#include "Puzzle.h"

// Input for a selection decision. Filled by StatsStore from Day 5 on.
struct SelectionContext {
    int playerRating = 1500;
    int ratingWindow = 200;
    QHash<QString, int> themeFailures;   // theme -> failed attempts
    QSet<QString> recentIds;             // puzzles not to repeat
};

// Strategy interface: picks the next puzzle from a pool.
class IPuzzleSelector {
public:
    virtual ~IPuzzleSelector() = default;
    virtual QString name() const = 0;
    // Returns a pointer into 'pool', or nullptr if the pool is empty.
    virtual const Puzzle* select(const QList<Puzzle>& pool, const SelectionContext& ctx) = 0;
};

namespace SelectorUtils {

// Entries matching 'pred' that were not played recently. If none, all matching entries.
template <typename Pred>
QList<const Puzzle*> candidates(const QList<Puzzle>& pool, const SelectionContext& ctx,
                                Pred pred) {
    QList<const Puzzle*> fresh;
    QList<const Puzzle*> all;
    for (const Puzzle& p : pool) {
        if (!pred(p)) continue;
        all.push_back(&p);
        if (!ctx.recentIds.contains(p.id)) fresh.push_back(&p);
    }
    return fresh.isEmpty() ? all : fresh;
}

inline const Puzzle* closestRating(const QList<const Puzzle*>& list, int rating) {
    if (list.isEmpty()) return nullptr;
    return *std::min_element(list.cbegin(), list.cend(),
        [rating](const Puzzle* a, const Puzzle* b) {
            return std::abs(a->rating - rating) < std::abs(b->rating - rating);
        });
}

}  // namespace SelectorUtils
