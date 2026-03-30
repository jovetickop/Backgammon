#pragma once

#include <QObject>
#include <QVector>
#include <QPoint>

/**
 * GameBridge - Qt Widgets 与 C++ 业务逻辑的桥接类
 * 负责将游戏核心逻辑暴露给 UI 层
 */
class GameBridge : public QObject
{
    Q_OBJECT

public:
    explicit GameBridge(QObject *parent = nullptr);
    ~GameBridge() = default;

    // 游戏控制
    void startGame();
    void resetGame();

    // 设置
    void setPlayerStarts(bool playerStarts);
    void setDifficulty(int depth);
    void setShowTop10(bool show);

    // 玩家落子
    void handlePlayerMove(int row, int col);

    // 对话框
    void showHistory();
    void showAiInfo();

    // 获取数据
    int getMoveCount();
    int getPlayerWinRate();
    int getAiWinRate();
    QVariantList getPlayerRateHistory();
    QVariantList getAiRateHistory();
    QVariantList getHighlightPositions();

    // 设置获胜连线
    void setHighlightPositions(const QVector<QPoint> &positions);

    // 获取棋盘数据
    const int (*getBoard())[15] { return m_board; }

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
