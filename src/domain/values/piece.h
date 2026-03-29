#pragma once

// 棋子类型值对象：不可变，表示棋盘上的棋子。
namespace game_core {

// 棋子类型枚举
enum class Piece : int {
    None = 0,   // 无子
    White = 1,  // 白子（AI）
    Black = 2   // 黑子（玩家）
};

// 获取对手棋子
constexpr Piece opponent(Piece piece) {
    if (piece == Piece::White) return Piece::Black;
    if (piece == Piece::Black) return Piece::White;
    return Piece::None;
}

} // namespace game_core
