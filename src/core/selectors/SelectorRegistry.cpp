#include "SelectorRegistry.h"

#include "ByRatingSelector.h"
#include "RandomSelector.h"
#include "WeakestThemeSelector.h"

SelectorRegistry SelectorRegistry::withDefaults() {
    SelectorRegistry r;
    r.add(QStringLiteral("Random"), [](quint32 seed) {
        return std::make_unique<RandomSelector>(seed);
    });
    r.add(QStringLiteral("ByRating"), [](quint32 seed) {
        return std::make_unique<ByRatingSelector>(seed);
    });
    r.add(QStringLiteral("WeakestTheme"), [](quint32 seed) {
        return std::make_unique<WeakestThemeSelector>(std::make_unique<ByRatingSelector>(seed));
    });
    return r;
}

void SelectorRegistry::add(const QString& name, Factory factory) {
    m_factories[name] = std::move(factory);
}

bool SelectorRegistry::contains(const QString& name) const {
    return m_factories.find(name) != m_factories.end();
}

QStringList SelectorRegistry::names() const {
    QStringList result;
    for (const auto& [name, factory] : m_factories) result.push_back(name);
    return result;
}

std::unique_ptr<IPuzzleSelector> SelectorRegistry::create(const QString& name,
                                                          quint32 seed) const {
    const auto it = m_factories.find(name);
    return it == m_factories.end() ? nullptr : it->second(seed);
}
