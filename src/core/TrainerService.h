#pragma once
#include <map>
#include <memory>
#include <optional>
#include <QRandomGenerator>
#include "PuzzleRepository.h"
#include "PuzzleSession.h"
#include "SelectorRegistry.h"
#include "StatsStore.h"
#include "TrainerTypes.h"

// Facade: the single entry point for QML, the client REST API and the admin API.
class TrainerService {
public:
    struct Options {
        QString dataPath;
        QString selectorName = QStringLiteral("ByRating");
        int ratingWindow = 200;
        std::optional<quint32> seed;   // fixed in tests, random otherwise
        int maxOpenSessions = 64;
    };

    explicit TrainerService(Options options);

    bool load(QString* error = nullptr);
    int puzzleCount() const;

    QStringList availableSelectors() const;
    QString selectorName() const;
    bool setSelector(const QString& name);   // false if the name is unknown
    void setRatingWindow(int window);

    std::optional<PuzzleView> startNext();
    MoveOutcome submitMove(const QString& sessionId, const QString& uci);

    StatsSnapshot stats() const;
    void resetStats();

private:
    quint32 nextSeed() { return m_rng.generate(); }
    void storeSession(const QString& id, PuzzleSession session);

    Options m_options;
    PuzzleRepository m_repo;
    SelectorRegistry m_registry;
    std::unique_ptr<IPuzzleSelector> m_selector;
    StatsStore m_stats;
    std::map<QString, PuzzleSession> m_sessions;
    QList<QString> m_sessionOrder;   // oldest first, for the open-session limit
    QRandomGenerator m_rng;
};
