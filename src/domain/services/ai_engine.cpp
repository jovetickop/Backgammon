#include "ai_engine.h"
#include "ComputerMove.h"
#include <algorithm>
#include <limits>
#include <cstring>

namespace game_core {

AIEngine::AIEngine()
    : search_depth_(4)
    , best_move_(Position::Invalid())
{
}

// 转换为ePiece
static ePiece toEPiece(Piece p) {
    if (p == Piece::White) return WHITE;
    if (p == Piece::Black) return BLACK;
    return NONE;
}

static Piece convertPiece(ePiece p) {
    if (p == WHITE) return Piece::White;
    if (p == BLACK) return Piece::Black;
    return Piece::None;
}

Position AIEngine::calculateBestMove(GameBoard& board, Piece aiPiece) {
    // 如果棋盘为空，返回中心点
    if (board.isEmpty()) {
        return Position(7, 7);
    }

    // 将GameBoard转换为ePiece数组供原有函数使用
    ePiece arrBoard[15][15];
    const auto& src = board.board();
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            arrBoard[i][j] = toEPiece(src[i][j]);
        }
    }

    // 使用原有ComputerMove逻辑
    ComputerMove computer;
    computer.MaxMinSearch(arrBoard, search_depth_);

    // 获取结果并更新棋盘
    Position result(computer.Y(), computer.X());
    if (result.isValid()) {
        best_move_ = result;
    }

    return result;
}

bool AIEngine::checkWin(const GameBoard& board, Piece piece) {
    return win_detector_.checkWin(board, piece);
}

std::vector<CandidateMove> AIEngine::generateCandidates(GameBoard& board, Piece piece) {
    return evaluator_.generateCandidates(board, piece);
}

int AIEngine::maxSearch(GameBoard& board, int depth, int alpha, int beta, Piece aiPiece) {
    // 终止条件：达到搜索深度或已分出胜负
    if (depth == 0) {
        return evaluator_.evaluate(board);
    }

    // 检查是否已获胜
    if (win_detector_.checkWin(board, aiPiece)) {
        return std::numeric_limits<int>::max() - depth;
    }

    Piece opponent = (aiPiece == Piece::White) ? Piece::Black : Piece::White;
    if (win_detector_.checkWin(board, opponent)) {
        return std::numeric_limits<int>::min() + depth;
    }

    // 生成候选点
    auto candidates = generateCandidates(board, aiPiece);

    // 限制候选点数量以提高效率
    if (candidates.size() > 10) {
        candidates.resize(10);
    }

    int maxScore = std::numeric_limits<int>::min();

    for (const auto& candidate : candidates) {
        // 尝试落子
        if (board.placePiece(candidate.position, aiPiece)) {
            // 递归搜索对手层
            int score = minSearch(board, depth - 1, alpha, beta, aiPiece);
            board.removePiece(candidate.position);

            maxScore = std::max(maxScore, score);
            alpha = std::max(alpha, score);

            if (beta <= alpha) {
                break;  // Beta剪枝
            }
        }
    }

    return maxScore;
}

int AIEngine::minSearch(GameBoard& board, int depth, int alpha, int beta, Piece aiPiece) {
    // 终止条件
    if (depth == 0) {
        return evaluator_.evaluate(board);
    }

    Piece opponent = (aiPiece == Piece::White) ? Piece::Black : Piece::White;

    // 检查是否已获胜
    if (win_detector_.checkWin(board, aiPiece)) {
        return std::numeric_limits<int>::max() - depth;
    }
    if (win_detector_.checkWin(board, opponent)) {
        return std::numeric_limits<int>::min() + depth;
    }

    // 生成候选点
    auto candidates = generateCandidates(board, opponent);

    // 限制候选点数量
    if (candidates.size() > 10) {
        candidates.resize(10);
    }

    int minScore = std::numeric_limits<int>::max();

    for (const auto& candidate : candidates) {
        // 对手落子
        if (board.placePiece(candidate.position, opponent)) {
            // 递归搜索AI层
            int score = maxSearch(board, depth - 1, alpha, beta, aiPiece);
            board.removePiece(candidate.position);

            minScore = std::min(minScore, score);
            beta = std::min(beta, score);

            if (beta <= alpha) {
                break;  // Alpha剪枝
            }
        }
    }

    return minScore;
}

} // namespace game_core
