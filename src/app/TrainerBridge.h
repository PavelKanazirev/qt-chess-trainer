#pragma once
#include <QObject>
#include "BoardModel.h"
#include "TrainerService.h"

// Adapter between QML and the TrainerService facade. Lives on the GUI thread.
class TrainerBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(BoardModel* board READ board CONSTANT)
    Q_PROPERTY(QString puzzleId READ puzzleId NOTIFY puzzleChanged)
    Q_PROPERTY(int puzzleRating READ puzzleRating NOTIFY puzzleChanged)
    Q_PROPERTY(QString themes READ themes NOTIFY puzzleChanged)
    Q_PROPERTY(bool whiteToMove READ whiteToMove NOTIFY puzzleChanged)
    Q_PROPERTY(int playerRating READ playerRating NOTIFY statsChanged)
    Q_PROPERTY(int solvedCount READ solvedCount NOTIFY statsChanged)
    Q_PROPERTY(int failedCount READ failedCount NOTIFY statsChanged)
    Q_PROPERTY(QString selector READ selector NOTIFY statsChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)

public:
    explicit TrainerBridge(TrainerService& service, QObject* parent = nullptr);

    BoardModel* board() const { return m_board; }
    QString puzzleId() const { return m_view.puzzleId; }
    int puzzleRating() const { return m_view.rating; }
    QString themes() const { return m_view.themes.join(QStringLiteral(", ")); }
    bool whiteToMove() const { return m_view.whiteToMove; }
    int playerRating() const { return m_stats.playerRating; }
    int solvedCount() const { return m_stats.solved; }
    int failedCount() const { return m_stats.failed; }
    QString selector() const { return m_stats.selector; }
    QString statusText() const { return m_status; }

    Q_INVOKABLE void nextPuzzle();

signals:
    void puzzleChanged();
    void statsChanged();
    void statusChanged();

private:
    void refreshStats();
    void setStatus(const QString& text);

    TrainerService& m_service;
    BoardModel* m_board;   // owned through QObject parenting
    PuzzleView m_view;
    StatsSnapshot m_stats;
    QString m_status;
};
