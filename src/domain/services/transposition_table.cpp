#include "transposition_table.h"
#include <cstring>

namespace game_core {

// ==================== ZobristHash ====================

ZobristHash::ZobristHash() {
    // 使用固定种子保证可重复性
    std::mt19937_64 rng(0xDEADBEEFCAFEBABEULL);
    for (int r = 0; r < GameBoard::BOARD_SIZE; ++r) {
        for (int c = 0; c < GameBoard::BOARD_SIZE; ++c) {
            table_[r][c][0] = 0; // Piece::None 不需要随机数
            table_[r][c][1] = rng(); // Piece::White
            table_[r][c][2] = rng(); // Piece::Black
        }
    }
}

uint64_t ZobristHash::compute(const GameBoard& board) const {
    uint64_t hash = 0;
    const auto& arr = board.board();
    for (int r = 0; r < GameBoard::BOARD_SIZE; ++r) {
        for (int c = 0; c < GameBoard::BOARD_SIZE; ++c) {
            Piece p = arr[r][c];
            if (p != Piece::None) {
                int idx = (p == Piece::White) ? 1 : 2;
                hash ^= table_[r][c][idx];
            }
        }
    }
    return hash;
}

uint64_t ZobristHash::update(uint64_t hash, int row, int col, Piece piece) const {
    // 落子或提子：异或同一随机数实现切换
    if (piece == Piece::None) return hash;
    int idx = (piece == Piece::White) ? 1 : 2;
    return hash ^ table_[row][col][idx];
}

// ==================== TranspositionTable ====================

TranspositionTable::TranspositionTable(std::size_t max_size)
    : max_size_(max_size)
    , hit_count_(0)
{
    table_.reserve(max_size_);
}

std::optional<TTEntry> TranspositionTable::probe(uint64_t hash, int required_depth) const {
    auto it = table_.find(hash);
    if (it == table_.end()) {
        return std::nullopt;
    }
    const TTEntry& entry = it->second;
    if (entry.depth >= required_depth) {
        ++hit_count_;
        return entry;
    }
    return std::nullopt;
}

void TranspositionTable::store(uint64_t hash, int score, int depth, TTFlag flag) {
    // 超过最大容量时直接覆写（简单替换策略）
    if (table_.size() >= max_size_) {
        // 当容量满时只替换深度更浅的条目，优先保留深度更大的搜索结果
        auto it = table_.find(hash);
        if (it != table_.end() && it->second.depth > depth) {
            return; // 已有更深的结果，不覆写
        }
    }
    table_[hash] = TTEntry{score, depth, flag};
}

void TranspositionTable::clear() {
    table_.clear();
    hit_count_ = 0;
}

} // namespace game_core
