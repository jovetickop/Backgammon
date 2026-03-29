#pragma once

#include "../values/values.h"
#include <array>
#include <vector>
#include <optional>

// 棋盘聚合根：管理15x15棋盘状态，是领域层的核心。
// 外部只能通过聚合根的方法来修改棋盘状态。
namespace game_core {

// 棋盘聚合根
class GameBoard {
public:
    // 常量：棋盘大小
    static constexpr int BOARD_SIZE = 15;

    // 构造函数：创建空棋盘
    GameBoard();

    // 在指定位置落子
    // 返回值：是否成功落子（失败原因：位置已有子或越界）
    bool placePiece(const Position& pos, Piece piece);

    // 移除指定位置的棋子（悔棋用）
    void removePiece(const Position& pos);

    // 获取指定位置的棋子
    Piece getPiece(const Position& pos) const;

    // 检查指定位置是否可以落子
    bool canPlace(const Position& pos) const;

    // 获取棋盘数组引用（供领域服务使用）
    const std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE>& board() const noexcept {
        return board_;
    }

    // 清除棋盘
    void reset();

    // 获取最后落子位置
    const Position& lastMove() const noexcept { return last_move_; }

    // 获取落子历史
    const std::vector<Move>& moveHistory() const noexcept { return move_history_; }

    // 获取当前手数
    int moveCount() const noexcept { return static_cast<int>(move_history_.size()); }

    // 检查棋盘是否为空
    bool isEmpty() const noexcept { return move_history_.empty(); }

    // 检查棋盘是否已满
    bool isFull() const noexcept {
        return move_count_ >= BOARD_SIZE * BOARD_SIZE;
    }

    // 获取某一方已落子数量
    int pieceCount(Piece piece) const;

private:
    // 内部状态
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> board_;  // 棋盘状态
    Position last_move_;                                            // 最后落子位置
    std::vector<Move> move_history_;                                // 落子历史
    int move_count_;                                                // 总手数
    int white_count_;                                               // 白子数量
    int black_count_;                                               // 黑子数量
};

} // namespace game_core
