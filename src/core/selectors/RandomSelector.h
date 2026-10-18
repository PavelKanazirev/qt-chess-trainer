#pragma once
#include <QRandomGenerator>
#include "IPuzzleSelector.h"

class RandomSelector : public IPuzzleSelector {
public:
    explicit RandomSelector(quint32 seed);
    QString name() const override { return QStringLiteral("Random"); }
    const Puzzle* select(const QList<Puzzle>& pool, const SelectionContext& ctx) override;

private:
    QRandomGenerator m_rng;
};
