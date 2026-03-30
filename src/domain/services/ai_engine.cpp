#include "ai_engine.h"
#include <algorithm>
#include <limits>

namespace game_core {

AIEngine::AIEngine()
    : search_depth_(6)       // 迭代加深最大深度上限
    , time_limit_ms_(2000)   // 默认 2 秒时间限制
    , error_rate_(0.0)       // 默认无错误
    , timed_out_(false)
    , best_move_(Position::Invalid())
    , rng_(std::random_device{}())
{
}

void AIEngine::setDifficultyProfile(const DifficultyProfile& profile) {
    // 根据难度配置更新搜索参数
    search_depth_  = profile.search_depth;
    time_limit_ms_ = profile.time_limit_ms;
    error_rate_    = profile.error_rate;
}

bool AIEngine::isTimeout() const {
    if (time_limit_ms_ <= 0) return false;
    auto elapsed = std::chrono::steady_clock::now() - search_start_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
           >= time_limit_ms_;
}

Position AIEngine::calculateBestMove(GameBoard& board, Piece aiPiece) {
    // 棋盘为空时落中心点
    if (board.isEmpty()) {
        return Position(7, 7);
    }

    // 开局库查询：开局阶段优先查表
    if (opening_book_.isLoaded()) {
        Position bookMove = opening_book_.lookup(board, aiPiece);
        if (bookMove.isValid()) {
            return bookMove;
        }
    }

    // 错误率模拟：按配置概率走随机候选步（简单难度人格化）
    if (error_rate_ > 0.0) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng_) < error_rate_) {
            auto candidates = evaluator_.generateCandidates(board, aiPiece);
            if (!candidates.empty()) {
                // 从候选步的后半段随机选一步（非最优），模拟失误
                int skip = static_cast<int>(candidates.size() / 2);
                std::uniform_int_distribution<int> idist(skip,
                    static_cast<int>(candidates.size()) - 1);
                return candidates[idist(rng_)].position;
            }
        }
    }

    // 记录搜索开始时间
    search_start_ = std::chrono::steady_clock::now();
    timed_out_ = false;
    tt_.resetStats();

    return iterativeDeepeningSearch(board, aiPiece);
}

Position AIEngine::iterativeDeepeningSearch(GameBoard& board, Piece aiPiece) {
    // 从深度1开始逐步加深，每次迭代完成后保存最佳走法用于下一轮移动排序
    Position best = Position::Invalid();

    for (int depth = 1; depth <= search_depth_; ++depth) {
        Position candidate = searchAtDepth(board, aiPiece, depth);

        if (timed_out_) {
            // 超时：若本次迭代有结果则用它，否则沿用上一次
            if (candidate.isValid()) {
                best = candidate;
            }
            break;
        }

        if (candidate.isValid()) {
            best = candidate;
        }
    }

    return best.isValid() ? best : Position(7, 7);
}

Position AIEngine::searchAtDepth(GameBoard& board, Piece aiPiece, int depth) {
    best_move_ = Position::Invalid();

    auto candidates = generateCandidates(board, aiPiece);
    if (candidates.size() > 12) candidates.resize(12);
    if (candidates.empty()) return Position(7, 7);

    int maxScore = std::numeric_limits<int>::min();
    int alpha    = std::numeric_limits<int>::min();
    const int beta = std::numeric_limits<int>::max();

    // 计算当前棋盘哈希
    uint64_t root_hash = zobrist_.compute(board);

    for (const auto& cand : candidates) {
        if (isTimeout()) {
            timed_out_ = true;
            break;
        }

        if (!board.placePiece(cand.position, aiPiece)) continue;

        uint64_t child_hash = zobrist_.update(root_hash, cand.position.row(),
                                               cand.position.col(), aiPiece);

        // 直接获胜则立即返回
        if (win_detector_.checkWin(board, aiPiece)) {
            board.removePiece(cand.position);
            return cand.position;
        }

        int s = minSearch(board, depth - 1, alpha, beta, aiPiece, child_hash);
        board.removePiece(cand.position);

        if (s > maxScore) {
            maxScore = s;
            best_move_ = cand.position;
        }
        alpha = std::max(alpha, s);
    }

    return best_move_;
}

bool AIEngine::checkWin(const GameBoard& board, Piece piece) {
    return win_detector_.checkWin(board, piece);
}

std::vector<CandidateMove> AIEngine::generateCandidates(GameBoard& board, Piece piece) {
    return evaluator_.generateCandidates(board, piece);
}

int AIEngine::maxSearch(GameBoard& board, int depth, int alpha, int beta,
                         Piece aiPiece, uint64_t hash) {
    // 超时检查
    if (isTimeout()) {
        timed_out_ = true;
        return evaluator_.evaluate(board);
    }

    // 终止条件
    if (depth == 0) {
        return evaluator_.evaluate(board);
    }

    // 置换表查询
    auto tt_entry = tt_.probe(hash, depth);
    if (tt_entry.has_value()) {
        const auto& e = tt_entry.value();
        if (e.flag == TTFlag::Exact) return e.score;
        if (e.flag == TTFlag::LowerBound) alpha = std::max(alpha, e.score);
        if (e.flag == TTFlag::UpperBound) beta  = std::min(beta,  e.score);
        if (alpha >= beta) return e.score;
    }

    // 检查胜负
    if (win_detector_.checkWin(board, aiPiece)) {
        return std::numeric_limits<int>::max() - depth;
    }
    Piece opponent = (aiPiece == Piece::White) ? Piece::Black : Piece::White;
    if (win_detector_.checkWin(board, opponent)) {
        return std::numeric_limits<int>::min() + depth;
    }

    auto candidates = generateCandidates(board, aiPiece);
    if (candidates.size() > 12) candidates.resize(12);

    int orig_alpha = alpha;
    int maxScore   = std::numeric_limits<int>::min();

    for (const auto& cand : candidates) {
        if (isTimeout()) {
            timed_out_ = true;
            break;
        }
        if (!board.placePiece(cand.position, aiPiece)) continue;
        uint64_t child_hash = zobrist_.update(hash, cand.position.row(),
                                               cand.position.col(), aiPiece);
        int s = minSearch(board, depth - 1, alpha, beta, aiPiece, child_hash);
        board.removePiece(cand.position);

        maxScore = std::max(maxScore, s);
        alpha    = std::max(alpha, s);
        if (alpha >= beta) break; // Beta 剪枝
    }

    // 存入置换表
    if (!timed_out_) {
        TTFlag flag;
        if      (maxScore <= orig_alpha) flag = TTFlag::UpperBound;
        else if (maxScore >= beta)       flag = TTFlag::LowerBound;
        else                             flag = TTFlag::Exact;
        tt_.store(hash, maxScore, depth, flag);
    }

    return maxScore;
}

int AIEngine::minSearch(GameBoard& board, int depth, int alpha, int beta,
                         Piece aiPiece, uint64_t hash) {
    // 超时检查
    if (isTimeout()) {
        timed_out_ = true;
        return evaluator_.evaluate(board);
    }

    // 终止条件
    if (depth == 0) {
        return evaluator_.evaluate(board);
    }

    // 置换表查询
    auto tt_entry = tt_.probe(hash, depth);
    if (tt_entry.has_value()) {
        const auto& e = tt_entry.value();
        if (e.flag == TTFlag::Exact) return e.score;
        if (e.flag == TTFlag::LowerBound) alpha = std::max(alpha, e.score);
        if (e.flag == TTFlag::UpperBound) beta  = std::min(beta,  e.score);
        if (alpha >= beta) return e.score;
    }

    Piece opponent = (aiPiece == Piece::White) ? Piece::Black : Piece::White;

    // 检查胜负
    if (win_detector_.checkWin(board, aiPiece)) {
        return std::numeric_limits<int>::max() - depth;
    }
    if (win_detector_.checkWin(board, opponent)) {
        return std::numeric_limits<int>::min() + depth;
    }

    auto candidates = generateCandidates(board, opponent);
    if (candidates.size() > 12) candidates.resize(12);

    int orig_beta = beta;
    int minScore  = std::numeric_limits<int>::max();

    for (const auto& cand : candidates) {
        if (isTimeout()) {
            timed_out_ = true;
            break;
        }
        if (!board.placePiece(cand.position, opponent)) continue;
        uint64_t child_hash = zobrist_.update(hash, cand.position.row(),
                                               cand.position.col(), opponent);
        int s = maxSearch(board, depth - 1, alpha, beta, aiPiece, child_hash);
        board.removePiece(cand.position);

        minScore = std::min(minScore, s);
        beta     = std::min(beta, s);
        if (beta <= alpha) break; // Alpha 剪枝
    }

    // 存入置换表
    if (!timed_out_) {
        TTFlag flag;
        if      (minScore >= orig_beta) flag = TTFlag::LowerBound;
        else if (minScore <= alpha)     flag = TTFlag::UpperBound;
        else                            flag = TTFlag::Exact;
        tt_.store(hash, minScore, depth, flag);
    }

    return minScore;
}

} // namespace game_core
