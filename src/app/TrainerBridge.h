#pragma once
#include <QObject>
#include <QStringList>
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
    Q_PROPERTY(QString feedback READ feedback NOTIFY statusChanged)
    Q_PROPERTY(bool finished READ finished NOTIFY statusChanged)
    Q_PROPERTY(QStringList availableSelectors READ availableSelectors CONSTANT)

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
    QString feedback() const { return m_feedback; }   // "", correct, illegal, wrong, solved
    bool finished() const { return m_finished; }
    QStringList availableSelectors() const { return m_service.availableSelectors(); }
    void setReplyDelay(int ms) { m_replyDelayMs = ms; }

    Q_INVOKABLE void nextPuzzle();
    Q_INVOKABLE void squareClicked(int row);
    Q_INVOKABLE bool setSelector(const QString& name);

signals:
    void puzzleChanged();
    void statsChanged();
    void statusChanged();

private:
    void playMove(const QString& uci);
    void showReply(const QString& fen, const QString& reply);
    bool isPromotion(int fromRow, int toRow) const;
    void refreshStats();
    void setFeedback(const QString& kind, const QString& text);

    TrainerService& m_service;
    BoardModel* m_board;   // owned through QObject parenting
    PuzzleView m_view;
    StatsSnapshot m_stats;
    QString m_status;
    QString m_feedback;
    bool m_finished = false;
    bool m_waitingForReply = false;
    int m_replyDelayMs = 400;
    int m_generation = 0;  // increments per puzzle; stale replies are ignored
};