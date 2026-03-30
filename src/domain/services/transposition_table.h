#pragma once

#include "../aggregates/game_board.h"
#include "../values/piece.h"
#include <array>
#include <cstdint>
#include <optional>
#include <random>
#include <unordered_map>

// 置换表与 Zobrist 哈希：避免重复计算，提升 AI 搜索效率。
namespace game_core {

// 置换表条目类型
enum class TTFlag {
    Exact,      // 精确值
    LowerBound, // Alpha 剪枝（下界）
    UpperBound  // Beta 剪枝（上界）
};

// 置换表条目
struct TTEntry {
    int   score;    // 评估分值
    int   depth;    // 搜索深度
    TTFlag flag;    // 条目类型
};

// Zobrist 哈希：将棋盘状态映射为 64 位哈希值。
// 每个位置×棋子类型预生成随机数，落子/提子通过异或实现 O(1) 更新。
class ZobristHash {
public:
    // 初始化随机数表
    ZobristHash();

    // 计算棋盘完整哈希（用于初始化）
    uint64_t compute(const GameBoard& board) const;

    // 增量更新：落子或提子时调用
    // 返回更新后的哈希值
    uint64_t update(uint64_t hash, int row, int col, Piece piece) const;

private:
    // 随机数表：[行][列][棋子类型(1=白,2=黑)]
    std::array<std::array<std::array<uint64_t, 3>, GameBoard::BOARD_SIZE>, GameBoard::BOARD_SIZE> table_;
};

// 置换表：用于缓存已搜索节点的评估结果。
class TranspositionTable {
public:
    // 构造函数：指定最大容量（条目数）
    explicit TranspositionTable(std::size_t max_size = 1 << 20);

    // 查询置换表
    // 返回匹配的条目（深度 >= required_depth 时有效）
    std::optional<TTEntry> probe(uint64_t hash, int required_depth) const;

    // 存入置换表
    void store(uint64_t hash, int score, int depth, TTFlag flag);

    // 清空置换表
    void clear();

    // 获取当前缓存命中次数（统计用）
    uint64_t hitCount() const { return hit_count_; }

    // 重置命中计数
    void resetStats() { hit_count_ = 0; }

private:
    std::unordered_map<uint64_t, TTEntry> table_; // 哈希表
    std::size_t max_size_;                         // 最大容量
    mutable uint64_t hit_count_;                   // 命中次数统计
};

} // namespace game_core
