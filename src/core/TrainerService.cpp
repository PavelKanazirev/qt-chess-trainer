#include "TrainerService.h"

#include <QUuid>

TrainerService::TrainerService(Options options)
    : m_options(std::move(options)),
      m_registry(SelectorRegistry::withDefaults()),
      m_rng(m_options.seed ? *m_options.seed : QRandomGenerator::global()->generate()) {
    if (!setSelector(m_options.selectorName)) setSelector(QStringLiteral("ByRating"));
}

bool TrainerService::load(QString* error) {
    if (!m_repo.loadFromFile(m_options.dataPath, error)) return false;
    if (m_repo.size() == 0) {
        if (error) *error = QStringLiteral("no valid puzzles in %1").arg(m_options.dataPath);
        return false;
    }
    return true;
}

int TrainerService::puzzleCount() const { return m_repo.size(); }

QStringList TrainerService::availableSelectors() const { return m_registry.names(); }

QString TrainerService::selectorName() const { return m_options.selectorName; }

bool TrainerService::setSelector(const QString& name) {
    auto selector = m_registry.create(name, nextSeed());
    if (!selector) return false;
    m_selector = std::move(selector);
    m_options.selectorName = name;
    return true;
}

void TrainerService::setRatingWindow(int window) {
    m_options.ratingWindow = std::max(0, window);
}

std::optional<PuzzleView> TrainerService::startNext() {
    if (!m_selector) return std::nullopt;
    const SelectionContext ctx = m_stats.selectionContext(m_options.ratingWindow);

    for (int attempt = 0; attempt < 5; ++attempt) {   // skip broken puzzles, if any
        const Puzzle* p = m_selector->select(m_repo.all(), ctx);
        if (!p) return std::nullopt;
        PuzzleSession session(*p);
        if (!session.isValid()) continue;

        PuzzleView view;
        view.sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        view.puzzleId = p->id;
        view.fen = session.fen();
        view.lastMove = session.firstOpponentMove();
        view.rating = p->rating;
        view.themes = p->themes;
        view.whiteToMove = view.fen.split(QLatin1Char(' ')).value(1) == QLatin1String("w");
        storeSession(view.sessionId, std::move(session));
        return view;
    }
    return std::nullopt;
}

MoveOutcome TrainerService::submitMove(const QString& sessionId, const QString& uci) {
    MoveOutcome out;
    out.playerRating = m_stats.playerRating();
    return out;   // implemented in the next commit
}

StatsSnapshot TrainerService::stats() const {
    StatsSnapshot s;
    s.playerRating = m_stats.playerRating();
    s.solved = m_stats.solvedCount();
    s.failed = m_stats.failedCount();
    s.themes = m_stats.themes();
    s.selector = m_options.selectorName;
    return s;
}

void TrainerService::resetStats() { m_stats.reset(); }

void TrainerService::storeSession(const QString& id, PuzzleSession session) {
    m_sessions.emplace(id, std::move(session));
    m_sessionOrder.push_back(id);
    while (m_sessionOrder.size() > m_options.maxOpenSessions)
        m_sessions.erase(m_sessionOrder.takeFirst());
}
