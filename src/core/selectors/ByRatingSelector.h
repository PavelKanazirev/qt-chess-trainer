#pragma once
#include <QRandomGenerator>
#include "IPuzzleSelector.h"

class ByRatingSelector : public IPuzzleSelector {
public:
    explicit ByRatingSelector(quint32 seed);
    QString name() const override { return QStringLiteral("ByRating"); }
    const Puzzle* select(const QList<Puzzle>& pool, const SelectionContext& ctx) override;

private:
    QRandomGenerator m_rng;
};
