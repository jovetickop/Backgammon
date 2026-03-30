#pragma once

#include <QObject>
#include "types.h"
#include "domain/services/difficulty_config.h"

// AI 计算 Worker：在独立 QThread 中执行 AI 搜索，完成后通过信号回传结果。
class AiWorker : public QObject
{
    Q_OBJECT

public:
    explicit AiWorker(QObject *parent = nullptr);

    // 设置棋盘状态和搜索深度（旧接口，保持兼容）
    void SetBoard(const ePiece board[15][15], int depth);

    // 设置棋盘状态和难度配置（新接口）
    void SetBoardWithProfile(const ePiece board[15][15],
                             const game_core::DifficultyProfile& profile);

public slots:
    // 在工作线程中执行 AI 搜索
    void Run();

signals:
    // AI 搜索完成后发出，传回最优落点
    void MoveReady(int row, int col);

private:
    ePiece m_board[15][15];                  // 棋盘快照（线程内部使用）
    int m_depth;                             // 搜索深度（旧接口）
    game_core::DifficultyProfile m_profile;  // 难度配置
    bool m_use_profile;                      // 是否使用难度配置
};
