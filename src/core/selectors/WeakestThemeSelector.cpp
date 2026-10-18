#include "WeakestThemeSelector.h"

WeakestThemeSelector::WeakestThemeSelector(std::unique_ptr<IPuzzleSelector> fallback)
    : m_fallback(std::move(fallback)) {}

QString WeakestThemeSelector::weakestTheme(const SelectionContext& ctx) {
    QString weakest;
    int worst = 0;
    for (auto it = ctx.themeFailures.cbegin(); it != ctx.themeFailures.cend(); ++it) {
        // most failures wins; ties broken by name so the result is deterministic
        if (it.value() > worst || (it.value() == worst && worst > 0 && it.key() < weakest)) {
            worst = it.value();
            weakest = it.key();
        }
    }
    return weakest;
}

const Puzzle* WeakestThemeSelector::select(const QList<Puzzle>& pool,
                                           const SelectionContext& ctx) {
    const QString theme = weakestTheme(ctx);
    if (!theme.isEmpty()) {
        const auto list = SelectorUtils::candidates(pool, ctx, [&theme](const Puzzle& p) {
            return p.themes.contains(theme);
        });
        if (!list.isEmpty()) return SelectorUtils::closestRating(list, ctx.playerRating);
    }
    return m_fallback ? m_fallback->select(pool, ctx) : nullptr;
}
