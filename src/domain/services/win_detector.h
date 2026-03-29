#pragma once

#include "../aggregates/game_board.h"
#include "../values/piece.h"

// 胜负检测领域服务：检测指定棋子是否已获胜。
namespace game_core {

// 胜负检测服务
class WinDetector {
public:
    // 判断指定棋子是否已获胜
    bool checkWin(const GameBoard& board, Piece piece);

    // 便捷方法：检查当前局面是否已分出胜负
    // 返回值：获胜方（None表示未分胜负）
    Piece checkWinner(const GameBoard& board);

private:
    // 四个方向的五连检测
    bool checkHorizontal(const GameBoard& board, Piece piece, int row, int col);
    bool checkVertical(const GameBoard& board, Piece piece, int row, int col);
    bool checkDiagonalLeft(const GameBoard& board, Piece piece, int row, int col);
    bool checkDiagonalRight(const GameBoard& board, Piece piece, int row, int col);
};

} // namespace game_core
