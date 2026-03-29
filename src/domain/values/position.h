#pragma once

#include <utility>

// 坐标值对象：不可变，表示棋盘上的一个位置。
namespace game_core {

// 坐标值对象
class Position {
public:
    // 常量：棋盘大小
    static constexpr int BOARD_SIZE = 15;

    // 构造函数
    constexpr Position(int row, int col) noexcept
        : row_(row), col_(col) {}

    // 默认构造函数（无效位置）
    Position() noexcept : row_(-1), col_(-1) {}

    // 获取行索引
    constexpr int row() const noexcept { return row_; }

    // 获取列索引
    constexpr int col() const noexcept { return col_; }

    // 判断是否在棋盘范围内
    constexpr bool isValid() const noexcept {
        return row_ >= 0 && row_ < BOARD_SIZE &&
               col_ >= 0 && col_ < BOARD_SIZE;
    }

    // 判断是否为有效落子位置（不含边界）
    constexpr bool isPlayable() const noexcept {
        return row_ > 0 && row_ < BOARD_SIZE - 1 &&
               col_ > 0 && col_ < BOARD_SIZE - 1;
    }

    // 静态工厂方法：创建无效位置
    static Position Invalid() noexcept { return Position(-1, -1); }

    // 相等比较
    constexpr bool operator==(const Position& other) const noexcept {
        return row_ == other.row_ && col_ == other.col_;
    }

    constexpr bool operator!=(const Position& other) const noexcept {
        return !(*this == other);
    }

    // 用于std::map/set的排序
    constexpr bool operator<(const Position& other) const noexcept {
        return row_ < other.row_ || (row_ == other.row_ && col_ < other.col_);
    }

private:
    int row_;  // 行索引 [0, 14]
    int col_;  // 列索引 [0, 14]
};

// 便捷别名
using Pos = Position;

} // namespace game_core
