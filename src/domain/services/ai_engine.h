#pragma once

#include "../aggregates/game_board.h"
#include "../values/piece.h"
#include "board_evaluator.h"
#include "difficulty_config.h"
#include "transposition_table.h"
#include "win_detector.h"
#include <atomic>
#include <chrono>
#include <optional>
#include <random>

// AI引擎领域服务：极大极小搜索 + Alpha-Beta剪枝 + 迭代加深 + 时间控制 + 置换表。
namespace game_core {

// AI引擎
class AIEngine {
public:
    // 构造函数
    AIEngine();

    // 设置搜索深度（迭代加深的最大深度上限）
    void setSearchDepth(int depth) { search_depth_ = depth; }

    // 获取搜索深度
    int searchDepth() const { return search_depth_; }

    // 设置时间限制（毫秒），0 表示不限时
    void setTimeLimitMs(int ms) { time_limit_ms_ = ms; }

    // 获取时间限制
    int timeLimitMs() const { return time_limit_ms_; }

    // 计算最佳落子位置（迭代加深 + 时间控制）
    // 返回最佳位置（如果没有合法位置返回 Invalid）
    Position calculateBestMove(GameBoard& board, Piece aiPiece);

    // 检查AI是否已经获胜
    bool checkWin(const GameBoard& board, Piece piece);

    // 设置难度配置（影响错误率、搜索深度、时间限制）
    void setDifficultyProfile(const DifficultyProfile& profile);

    // 清空置换表（新局开始时调用）
    void clearTranspositionTable() { tt_.clear(); }

    // 获取置换表命中次数（用于调试/统计）
    uint64_t ttHitCount() const { return tt_.hitCount(); }

private:
    // 生成候选落子点
    std::vector<CandidateMove> generateCandidates(GameBoard& board, Piece piece);

    // 迭代加深搜索入口：在时间限制内逐层加深
    Position iterativeDeepeningSearch(GameBoard& board, Piece aiPiece);

    // 固定深度搜索（单次迭代）
    // 返回最佳走法，若超时返回 Invalid
    Position searchAtDepth(GameBoard& board, Piece aiPiece, int depth);

    // 最大层搜索（AI回合）
    int maxSearch(GameBoard& board, int depth, int alpha, int beta,
                  Piece aiPiece, uint64_t hash);

    // 最小层搜索（对手回合）
    int minSearch(GameBoard& board, int depth, int alpha, int beta,
                  Piece aiPiece, uint64_t hash);

    // 检查是否已超时
    bool isTimeout() const;

    int search_depth_;             // 最大搜索深度上限
    int time_limit_ms_;            // 时间限制（毫秒），0 表示不限时
    double error_rate_;            // 错误率：走随机步的概率 [0.0, 1.0]
    bool timed_out_;               // 是否已超时标志

    std::chrono::steady_clock::time_point search_start_; // 搜索开始时间

    WinDetector win_detector_;    // 胜负检测
    BoardEvaluator evaluator_;     // 局面评估
    Position best_move_;           // 当前最佳落子位置

    ZobristHash zobrist_;          // Zobrist 哈希
    TranspositionTable tt_;        // 置换表
    std::mt19937 rng_;             // 随机数生成器（用于错误率模拟）
};

} // namespace game_core
