#include "AiWorker.h"
#include "domain/services/ai_engine.h"
#include "domain/aggregates/game_board.h"
#include <cstring>

AiWorker::AiWorker(QObject *parent)
    : QObject(parent)
    , m_depth(8)
{
    memset(m_board, 0, sizeof(m_board));
}

// 在主线程调用：拷贝棋盘状态和搜索深度到 Worker。
void AiWorker::SetBoard(const ePiece board[15][15], int depth)
{
    memcpy(m_board, board, sizeof(m_board));
    m_depth = depth;
}

// 在工作线程中执行：搜索最优落点并通过信号回传。
void AiWorker::Run()
{
    // 将 ePiece 棋盘转换为 GameBoard
    game_core::GameBoard gboard;
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j)
            if (m_board[i][j] != NONE) {
                game_core::Piece p = (m_board[i][j] == WHITE) ? game_core::Piece::White : game_core::Piece::Black;
                gboard.placePiece(game_core::Position(i, j), p);
            }

    game_core::AIEngine engine;
    engine.setSearchDepth(m_depth);
    game_core::Position best = engine.calculateBestMove(gboard, game_core::Piece::White);

    emit MoveReady(best.row(), best.col());
}
