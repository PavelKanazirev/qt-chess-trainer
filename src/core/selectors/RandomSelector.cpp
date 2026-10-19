#include "RandomSelector.h"

RandomSelector::RandomSelector(quint32 seed) : m_rng(seed) {}

const Puzzle* RandomSelector::select(const QList<Puzzle>& pool, const SelectionContext& ctx) {
    const auto list = SelectorUtils::candidates(pool, ctx, [](const Puzzle&) { return true; });
    if (list.isEmpty()) return nullptr;
    return list.at(m_rng.bounded(static_cast<quint32>(list.size())));
}
