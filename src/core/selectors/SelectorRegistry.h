#pragma once
#include <functional>
#include <map>
#include <memory>
#include <QStringList>
#include "IPuzzleSelector.h"

// Creates selectors by name, so the active strategy can come from configuration.
class SelectorRegistry {
public:
    using Factory = std::function<std::unique_ptr<IPuzzleSelector>(quint32 seed)>;

    static SelectorRegistry withDefaults();   // Random, ByRating, WeakestTheme

    void add(const QString& name, Factory factory);
    bool contains(const QString& name) const;
    QStringList names() const;
    std::unique_ptr<IPuzzleSelector> create(const QString& name, quint32 seed) const;

private:
    std::map<QString, Factory> m_factories;
};
