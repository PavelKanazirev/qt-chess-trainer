#include "PuzzleRepository.h"

#include <QFile>
#include <QTextStream>
#include "PuzzleParser.h"

bool PuzzleRepository::loadFromFile(const QString& path, QString* error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = file.errorString();
        return false;
    }

    m_puzzles.clear();
    m_index.clear();
    m_skipped = 0;

    QTextStream in(&file);
    bool firstLine = true;
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (firstLine) {
            firstLine = false;
            if (line.startsWith(QStringLiteral("PuzzleId"))) continue;  // header row
        }
        auto puzzle = PuzzleParser::parseLine(line);
        if (!puzzle || m_index.contains(puzzle->id)) {
            ++m_skipped;
            continue;
        }
        m_index.insert(puzzle->id, static_cast<int>(m_puzzles.size()));
        m_puzzles.push_back(std::move(*puzzle));
    }
    return true;
}

const Puzzle* PuzzleRepository::findById(const QString& id) const {
    const auto it = m_index.constFind(id);
    return it == m_index.constEnd() ? nullptr : &m_puzzles[it.value()];
}
