#include "ByRatingSelector.h"

ByRatingSelector::ByRatingSelector(quint32 seed) : m_rng(seed) {}

const Puzzle* ByRatingSelector::select(const QList<Puzzle>& pool, const SelectionContext& ctx) {
    const auto inWindow = SelectorUtils::candidates(pool, ctx, [&ctx](const Puzzle& p) {
        return std::abs(p.rating - ctx.playerRating) <= ctx.ratingWindow;
    });
    if (!inWindow.isEmpty())
        return inWindow.at(m_rng.bounded(static_cast<quint32>(inWindow.size())));

    const auto any = SelectorUtils::candidates(pool, ctx, [](const Puzzle&) { return true; });
    return SelectorUtils::closestRating(any, ctx.playerRating);
}
