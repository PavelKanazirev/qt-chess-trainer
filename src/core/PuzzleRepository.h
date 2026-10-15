#pragma once
#include <QHash>
#include <QList>
#include <QString>
#include "Puzzle.h"

class PuzzleRepository {
public:
    // Loads puzzles from a CSV file, replacing current contents.
    // Invalid or duplicate rows are skipped and counted.
    bool loadFromFile(const QString& path, QString* error = nullptr);

    int size() const { return static_cast<int>(m_puzzles.size()); }
    int skippedCount() const { return m_skipped; }
    const QList<Puzzle>& all() const { return m_puzzles; }
    const Puzzle* findById(const QString& id) const;

private:
    QList<Puzzle> m_puzzles;
    QHash<QString, int> m_index;
    int m_skipped = 0;
};
