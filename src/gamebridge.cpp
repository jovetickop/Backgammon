#include "gamebridge.h"
#include <QDebug>
#include <QVariantList>

GameBridge::GameBridge(QObject *parent)
    : QObject(parent)
{
    // 初始化棋盘
    clearBoard();
}

void GameBridge::clearBoard()
{
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            m_board[i][j] = 0;
        }
    }
    m_moveCount = 0;
    m_playerWinRate = 50;
    m_aiWinRate = 50;
    m_highlightPositions.clear();
    m_playerRateHistory.clear();
    m_aiRateHistory.clear();
}

void GameBridge::startGame()
{
    clearBoard();
    m_gameStarted = true;
    emit gameStarted();
    emit moveCountChanged(m_moveCount);
    emit winRateUpdated(m_playerWinRate, m_aiWinRate);

    // 如果 AI 先手，AI 走第一步
    if (!m_playerStarts) {
        aiMove();
    }
}

void GameBridge::resetGame()
{
    clearBoard();
    m_gameStarted = false;
    emit gameReset();
}

void GameBridge::setPlayerStarts(bool playerStarts)
{
    m_playerStarts = playerStarts;
}

void GameBridge::setDifficulty(int depth)
{
    m_difficulty = depth;
}

void GameBridge::setShowTop10(bool show)
{
    m_showTop10 = show;
}

void GameBridge::handlePlayerMove(int row, int col)
{
    if (!m_gameStarted) return;
    if (row < 0 || row >= 15 || col < 0 || col >= 15) return;
    if (m_board[row][col] != 0) return;

    // 玩家落子
    placePiece(row, col, true);
    emit movePlaced(row, col, true);

    // 检查玩家是否获胜
    if (checkWinner() == 1) {
        m_playerWins++;
        m_totalGames++;
        m_gameStarted = false;
        emit gameOver(1, m_highlightPositions);
        emit statsUpdated(m_totalGames, m_playerWins, m_aiWins,
                         m_totalGames > 0 ? (m_playerWins * 100 / m_totalGames) : 0,
                         m_totalGames > 0 ? (m_aiWins * 100 / m_totalGames) : 0);
        return;
    }

    // AI 落子
    aiMove();
}

void GameBridge::placePiece(int row, int col, bool isPlayer)
{
    m_board[row][col] = isPlayer ? 1 : 2;
    m_moveCount++;
    emit moveCountChanged(m_moveCount);
    updateWinRate();
}

void GameBridge::aiMove()
{
    // 简单的 AI：找空位
    // TODO: 集成完整的 AI 算法
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (m_board[i][j] == 0) {
                placePiece(i, j, false);
                emit movePlaced(i, j, false);

                // 检查 AI 是否获胜
                if (checkWinner() == 2) {
                    m_aiWins++;
                    m_totalGames++;
                    m_gameStarted = false;
                    emit gameOver(2, m_highlightPositions);
                    emit statsUpdated(m_totalGames, m_playerWins, m_aiWins,
                                     m_totalGames > 0 ? (m_playerWins * 100 / m_totalGames) : 0,
                                     m_totalGames > 0 ? (m_aiWins * 100 / m_totalGames) : 0);
                }
                return;
            }
        }
    }
}

int GameBridge::checkWinner()
{
    // 简化版：检查是否已有5子连珠
    // TODO: 集成完整的胜负判定逻辑
    return 0;
}

void GameBridge::updateWinRate()
{
    // 记录胜率历史
    m_playerRateHistory.append(m_playerWinRate);
    m_aiRateHistory.append(m_aiWinRate);

    // 简化版：根据手数更新胜率
    if (m_moveCount == 0) {
        m_playerWinRate = 50;
        m_aiWinRate = 50;
    } else {
        // 越到后面越准确
        int factor = qMin(m_moveCount * 5, 50);
        if (m_playerStarts) {
            m_playerWinRate = 50 + factor / 2;
        } else {
            m_playerWinRate = 50 - factor / 2;
        }
        m_aiWinRate = 100 - m_playerWinRate;
    }
    emit winRateUpdated(m_playerWinRate, m_aiWinRate);
}

void GameBridge::showHistory()
{
    // TODO: 显示历史对局对话框
    qDebug() << "Show history";
}

void GameBridge::showAiInfo()
{
    // TODO: 显示 AI 说明对话框
    qDebug() << "Show AI info";
}

int GameBridge::getMoveCount()
{
    return m_moveCount;
}

int GameBridge::getPlayerWinRate()
{
    return m_playerWinRate;
}

int GameBridge::getAiWinRate()
{
    return m_aiWinRate;
}

QVariantList GameBridge::getPlayerRateHistory()
{
    QVariantList list;
    for (int rate : m_playerRateHistory) {
        list.append(rate);
    }
    return list;
}

QVariantList GameBridge::getAiRateHistory()
{
    QVariantList list;
    for (int rate : m_aiRateHistory) {
        list.append(rate);
    }
    return list;
}

void GameBridge::setHighlightPositions(const QVector<QPoint> &positions)
{
    m_highlightPositions = positions;
}

QVariantList GameBridge::getHighlightPositions()
{
    QVariantList list;
    for (const QPoint &pos : m_highlightPositions) {
        QVariantMap map;
        map["row"] = pos.y();
        map["col"] = pos.x();
        list.append(map);
    }
    return list;
}
