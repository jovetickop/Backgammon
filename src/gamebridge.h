#pragma once

#include <QObject>
#include <QVector>
#include <QPoint>
#include <QQmlEngine>

/**
 * GameBridge - QML 与 C++ 业务逻辑的桥接类
 * 负责将游戏核心逻辑暴露给 QML 界面
 */
class GameBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit GameBridge(QObject *parent = nullptr);
    ~GameBridge() = default;

    // 游戏控制
    Q_INVOKABLE void startGame();
    Q_INVOKABLE void resetGame();

    // 设置
    Q_INVOKABLE void setPlayerStarts(bool playerStarts);
    Q_INVOKABLE void setDifficulty(int depth);
    Q_INVOKABLE void setShowTop10(bool show);

    // 玩家落子
    Q_INVOKABLE void handlePlayerMove(int row, int col);

    // 对话框
    Q_INVOKABLE void showHistory();
    Q_INVOKABLE void showAiInfo();

    // 获取数据（供 QML 调用）
    Q_INVOKABLE int getMoveCount();
    Q_INVOKABLE int getPlayerWinRate();
    Q_INVOKABLE int getAiWinRate();
    Q_INVOKABLE QVariantList getPlayerRateHistory();
    Q_INVOKABLE QVariantList getAiRateHistory();
    Q_INVOKABLE QVariantList getHighlightPositions();

    // 设置获胜连线
    Q_INVOKABLE void setHighlightPositions(const QVector<QPoint> &positions);

signals:
    // 游戏状态变化信号
    void gameStarted();
    void gameReset();
    void statsUpdated(int totalGames, int playerWins, int aiWins, int playerRate, int aiRate);
    void movePlaced(int row, int col, bool isPlayer);
    void moveCountChanged(int count);
    void winRateUpdated(int playerRate, int aiRate);
    void gameOver(int winner, const QVector<QPoint> &positions);

private:
    // 游戏数据
    int m_playerWins = 0;
    int m_aiWins = 0;
    int m_totalGames = 0;
    int m_moveCount = 0;
    int m_playerWinRate = 50;
    int m_aiWinRate = 50;
    bool m_playerStarts = true;
    int m_difficulty = 3;
    bool m_showTop10 = false;
    bool m_gameStarted = false;

    // 棋盘数据 (15x15)
    int m_board[15][15];

    // 胜率历史
    QVector<int> m_playerRateHistory;
    QVector<int> m_aiRateHistory;

    // 获胜连线位置
    QVector<QPoint> m_highlightPositions;

    // 私有方法
    void clearBoard();
    void updateWinRate();
    void placePiece(int row, int col, bool isPlayer);
    void aiMove();
    int checkWinner();
};
