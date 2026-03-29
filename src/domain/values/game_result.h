#pragma once

#include "piece.h"

// 游戏结果值对象：不可变，表示对局结果。
namespace game_core {

// 游戏结果枚举
enum class GameResult {
    None,    // 未分胜负（对局进行中）
    White,   // 白方获胜
    Black,   // 黑方获胜
    Draw     // 平局
};

// 从棋子类型获取游戏结果
constexpr GameResult resultFromPiece(Piece winner) {
    if (winner == Piece::White) return GameResult::White;
    if (winner == Piece::Black) return GameResult::Black;
    return GameResult::None;
}

// 判断是否有玩家获胜
constexpr bool hasWinner(GameResult result) {
    return result == GameResult::White || result == GameResult::Black;
}

} // namespace game_core
