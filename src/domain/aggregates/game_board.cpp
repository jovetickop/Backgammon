#include "game_board.h"

namespace game_core {

GameBoard::GameBoard()
    : last_move_(Position::Invalid())
    , move_count_(0)
    , white_count_(0)
    , black_count_(0)
{
    // 初始化空棋盘
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board_[i][j] = Piece::None;
        }
    }
}

bool GameBoard::placePiece(const Position& pos, Piece piece) {
    // 检查位置有效性
    if (!pos.isValid() || piece == Piece::None) {
        return false;
    }

    // 检查位置是否为空
    if (board_[pos.row()][pos.col()] != Piece::None) {
        return false;
    }

    // 落子
    board_[pos.row()][pos.col()] = piece;
    last_move_ = pos;
    move_history_.emplace_back(pos, piece);
    ++move_count_;

    if (piece == Piece::White) {
        ++white_count_;
    } else if (piece == Piece::Black) {
        ++black_count_;
    }

    return true;
}

void GameBoard::removePiece(const Position& pos) {
    if (!pos.isValid()) {
        return;
    }

    Piece piece = board_[pos.row()][pos.col()];
    if (piece == Piece::None) {
        return;
    }

    // 移除棋子
    board_[pos.row()][pos.col()] = Piece::None;
    --move_count_;

    if (piece == Piece::White) {
        --white_count_;
    } else if (piece == Piece::Black) {
        --black_count_;
    }

    // 更新最后落子位置
    if (!move_history_.empty()) {
        move_history_.pop_back();
        if (!move_history_.empty()) {
            last_move_ = move_history_.back().position();
        } else {
            last_move_ = Position::Invalid();
        }
    }
}

Piece GameBoard::getPiece(const Position& pos) const {
    if (!pos.isValid()) {
        return Piece::None;
    }
    return board_[pos.row()][pos.col()];
}

bool GameBoard::canPlace(const Position& pos) const {
    if (!pos.isValid()) {
        return false;
    }
    return board_[pos.row()][pos.col()] == Piece::None;
}

void GameBoard::reset() {
    // 清空棋盘
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board_[i][j] = Piece::None;
        }
    }

    last_move_ = Position::Invalid();
    move_history_.clear();
    move_count_ = 0;
    white_count_ = 0;
    black_count_ = 0;
}

int GameBoard::pieceCount(Piece piece) const {
    if (piece == Piece::White) return white_count_;
    if (piece == Piece::Black) return black_count_;
    return 0;
}

} // namespace game_core
