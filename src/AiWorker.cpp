#include "AiWorker.h"
#include "domain/services/ai_engine.h"
#include "domain/aggregates/game_board.h"
#include <QCoreApplication>
#include <cstring>

AiWorker::AiWorker(QObject *parent)
    : QObject(parent)
    , m_depth(8)
    , m_profile{2, "普通", 4, 1000, 0.10, 0.6, 0.6, "默认"}
    , m_use_profile(false)
{
    memset(m_board, 0, sizeof(m_board));
}

// 在主线程调用：拷贝棋盘状态和搜索深度到 Worker（旧接口）。
void AiWorker::SetBoard(const ePiece board[15][15], int depth)
{
    memcpy(m_board, board, sizeof(m_board));
    m_depth = depth;
    m_use_profile = false;
}

// 在主线程调用：拷贝棋盘状态和难度配置到 Worker（新接口）。
void AiWorker::SetBoardWithProfile(const ePiece board[15][15],
                                    const game_core::DifficultyProfile& profile)
{
    memcpy(m_board, board, sizeof(m_board));
    m_profile = profile;
    m_use_profile = true;
}

// 在工作线程中执行：搜索最优落点并通过信号回传。
void AiWorker::Run()
{
    // 将 ePiece 棋盘转换为 GameBoard
    game_core::GameBoard gboard;
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j)
            if (m_board[i][j] != NONE) {
                game_core::Piece p = (m_board[i][j] == WHITE)
                    ? game_core::Piece::White : game_core::Piece::Black;
                gboard.placePiece(game_core::Position(i, j), p);
            }

    game_core::AIEngine engine;
    // 加载开局库（从可执行文件同级目录的 resources 子目录查找）
    {
        QString bookPath = QCoreApplication::applicationDirPath() + "/resources/opening_book.json";
        engine.loadOpeningBook(bookPath);
    }
    if (m_use_profile) {
        // 使用难度配置（T016 人格化）
        engine.setDifficultyProfile(m_profile);
    } else {
        // 兼容旧接口：仅设置深度
        engine.setSearchDepth(m_depth);
    }
    game_core::Position best = engine.calculateBestMove(gboard, game_core::Piece::White);

    emit MoveReady(best.row(), best.col());
}
