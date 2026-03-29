#pragma once

#include "../aggregates/game_board.h"
#include "../values/piece.h"
#include "board_evaluator.h"
#include "win_detector.h"
#include <optional>

// AI引擎领域服务：极大极小搜索 + Alpha-Beta剪枝。
namespace game_core {

// AI引擎
class AIEngine {
public:
    // 构造函数
    AIEngine();

    // 设置搜索深度
    void setSearchDepth(int depth) { search_depth_ = depth; }

    // 获取搜索深度
    int searchDepth() const { return search_depth_; }

    // 计算最佳落子位置
    // 返回最佳位置（如果没有合法位置返回Invalid）
    Position calculateBestMove(GameBoard& board, Piece aiPiece);

    // 检查AI是否已经获胜
    bool checkWin(const GameBoard& board, Piece piece);

private:
    // 生成候选落子点
    std::vector<CandidateMove> generateCandidates(GameBoard& board, Piece piece);

    // 最大层搜索（AI回合）
    int maxSearch(GameBoard& board, int depth, int alpha, int beta, Piece aiPiece);

    // 最小层搜索（对手回合）
    int minSearch(GameBoard& board, int depth, int alpha, int beta, Piece aiPiece);

    int search_depth_;           // 搜索深度
    WinDetector win_detector_;  // 胜负检测
    BoardEvaluator evaluator_;   // 局面评估
    Position best_move_;         // 当前最佳落子位置
};

} // namespace game_core
