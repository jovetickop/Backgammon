#include "win_detector.h"

namespace game_core {

bool WinDetector::checkWin(const GameBoard& board, Piece piece) {
    // 全棋盘扫描，支持任意棋盘状态（非单步增量）
    const auto& arr = board.board();
    for (int r = 0; r < GameBoard::BOARD_SIZE; ++r) {
        for (int c = 0; c < GameBoard::BOARD_SIZE; ++c) {
            if (arr[r][c] == piece) {
                if (checkHorizontal(board, piece, r, c) ||
                    checkVertical(board, piece, r, c) ||
                    checkDiagonalLeft(board, piece, r, c) ||
                    checkDiagonalRight(board, piece, r, c))
                    return true;
            }
        }
    }
    return false;
}

Piece WinDetector::checkWinner(const GameBoard& board) {
    // 检查白方是否获胜
    if (checkWin(board, Piece::White)) {
        return Piece::White;
    }

    // 检查黑方是否获胜
    if (checkWin(board, Piece::Black)) {
        return Piece::Black;
    }

    return Piece::None;
}

bool WinDetector::checkHorizontal(const GameBoard& board, Piece piece, int row, int col) {
    // 横向检测：检查当前位置向左和向右各4个位置是否形成五连
    const auto& arr = board.board();

    // 找到连续同色棋子的起始位置
    int count = 1;  // 当前落子算1个

    // 向左检测
    for (int i = col - 1; i >= 0 && arr[row][i] == piece; --i) {
        ++count;
    }

    // 向右检测
    for (int i = col + 1; i < GameBoard::BOARD_SIZE && arr[row][i] == piece; ++i) {
        ++count;
    }

    return count >= 5;
}

bool WinDetector::checkVertical(const GameBoard& board, Piece piece, int row, int col) {
    // 纵向检测
    const auto& arr = board.board();

    int count = 1;

    // 向上检测
    for (int i = row - 1; i >= 0 && arr[i][col] == piece; --i) {
        ++count;
    }

    // 向下检测
    for (int i = row + 1; i < GameBoard::BOARD_SIZE && arr[i][col] == piece; ++i) {
        ++count;
    }

    return count >= 5;
}

bool WinDetector::checkDiagonalLeft(const GameBoard& board, Piece piece, int row, int col) {
    // 左下方向斜线检测（从右上到左下的斜线）
    const auto& arr = board.board();

    int count = 1;

    // 向左上检测
    int i = row - 1;
    int j = col + 1;
    while (i >= 0 && j < GameBoard::BOARD_SIZE && arr[i][j] == piece) {
        ++count;
        --i;
        ++j;
    }

    // 向右下检测
    i = row + 1;
    j = col - 1;
    while (i < GameBoard::BOARD_SIZE && j >= 0 && arr[i][j] == piece) {
        ++count;
        ++i;
        --j;
    }

    return count >= 5;
}

bool WinDetector::checkDiagonalRight(const GameBoard& board, Piece piece, int row, int col) {
    // 右下方向斜线检测（从左上到右下的斜线）
    const auto& arr = board.board();

    int count = 1;

    // 向左上检测
    int i = row - 1;
    int j = col - 1;
    while (i >= 0 && j >= 0 && arr[i][j] == piece) {
        ++count;
        --i;
        --j;
    }

    // 向右下检测
    i = row + 1;
    j = col + 1;
    while (i < GameBoard::BOARD_SIZE && j < GameBoard::BOARD_SIZE && arr[i][j] == piece) {
        ++count;
        ++i;
        ++j;
    }

    return count >= 5;
}

} // namespace game_core
