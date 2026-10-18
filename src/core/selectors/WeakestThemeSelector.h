#pragma once
#include <memory>
#include "IPuzzleSelector.h"

class WeakestThemeSelector : public IPuzzleSelector {
public:
    explicit WeakestThemeSelector(std::unique_ptr<IPuzzleSelector> fallback);
    QString name() const override { return QStringLiteral("WeakestTheme"); }
    const Puzzle* select(const QList<Puzzle>& pool, const SelectionContext& ctx) override;

private:
    static QString weakestTheme(const SelectionContext& ctx);
    std::unique_ptr<IPuzzleSelector> m_fallback;
};
