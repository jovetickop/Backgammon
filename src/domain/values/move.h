#pragma once

#include "position.h"
#include "piece.h"

// 落子值对象：不可变，表示一步落子动作。
namespace game_core {

// 落子值对象
class Move {
public:
    // 构造函数
    Move(const Position& position, Piece piece) noexcept
        : position_(position), piece_(piece) {}

    // 获取落子位置
    const Position& position() const noexcept { return position_; }

    // 获取棋子类型
    Piece piece() const noexcept { return piece_; }

    // 判断是否有效
    bool isValid() const noexcept {
        return position_.isValid() && piece_ != Piece::None;
    }

    // 相等比较
    bool operator==(const Move& other) const noexcept {
        return position_ == other.position_ && piece_ == other.piece_;
    }

    bool operator!=(const Move& other) const noexcept {
        return !(*this == other);
    }

private:
    const Position position_;  // 落子位置
    const Piece piece_;        // 棋子类型
};

} // namespace game_core
