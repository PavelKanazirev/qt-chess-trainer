#pragma once
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <mutex>
#include <QRandomGenerator>
#include "PuzzleRepository.h"
#include "PuzzleSession.h"
#include "SelectorRegistry.h"
#include "StatsStore.h"
#include "TrainerTypes.h"


// Facade: the single entry point for QML, the client REST API and the admin API.
class TrainerService {
public:
    using ChangeListener = std::function<void()>;
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
    // Called after statistics or settings change, on the thread that made the change.
    // It must not call back into TrainerService synchronously.
    void setChangeListener(ChangeListener listener);
    int ratingWindow() const;

    std::optional<PuzzleView> startNext();
    MoveOutcome submitMove(const QString& sessionId, const QString& uci);

    StatsSnapshot stats() const;
    void resetStats();

private:
    quint32 nextSeed() { return m_rng.generate(); }
    void storeSession(const QString& id, PuzzleSession session);
    void notifyChanged();

    Options m_options;
    PuzzleRepository m_repo;
    SelectorRegistry m_registry;
    std::unique_ptr<IPuzzleSelector> m_selector;
    StatsStore m_stats;
    std::map<QString, PuzzleSession> m_sessions;
    QList<QString> m_sessionOrder;   // oldest first, for the open-session limit
    QRandomGenerator m_rng;
    mutable std::mutex m_mutex;
    std::mutex m_listenerMutex;   // also lets setChangeListener({}) wait for a running call
    ChangeListener m_listener;
};
