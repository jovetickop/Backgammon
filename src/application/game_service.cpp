#include "game_service.h"

namespace game_core {

GameService::GameService()
    : state_(GameState::Idle)
    , current_player_(Piece::Black)  // 默认黑方先行
    , result_(GameResult::None)
{
    // 默认AI搜索深度为4
    ai_engine_.setSearchDepth(4);
}

void GameService::startNewGame() {
    // 重置棋盘
    board_.reset();

    // 重置状态
    state_ = GameState::Playing;
    current_player_ = Piece::Black;  // 黑方先行
    result_ = GameResult::None;
}

bool GameService::playerMove(const Position& pos) {
    // 检查游戏状态
    if (state_ != GameState::Playing) {
        return false;
    }

    // 检查是否是玩家回合
    if (current_player_ != Piece::Black) {  // 黑方代表玩家
        return false;
    }

    // 检查位置是否可落子
    if (!board_.canPlace(pos)) {
        return false;
    }

    // 玩家落子
    if (!board_.placePiece(pos, Piece::Black)) {
        return false;
    }

    // 检查是否获胜
    if (win_detector_.checkWin(board_, Piece::Black)) {
        result_ = GameResult::Black;
        state_ = GameState::Finished;
        return true;
    }

    // 切换到AI回合
    switchPlayer();

    return true;
}

Position GameService::aiMove() {
    // 检查游戏状态
    if (state_ != GameState::Playing) {
        return Position::Invalid();
    }

    // 检查是否是AI回合
    if (current_player_ != Piece::White) {  // 白方代表AI
        return Position::Invalid();
    }

    // AI计算最佳落子
    Position bestPos = ai_engine_.calculateBestMove(board_, Piece::White);

    // 检查返回位置是否有效
    if (!bestPos.isValid()) {
        return Position::Invalid();
    }

    // AI落子
    if (!board_.placePiece(bestPos, Piece::White)) {
        return Position::Invalid();
    }

    // 检查是否获胜
    if (win_detector_.checkWin(board_, Piece::White)) {
        result_ = GameResult::White;
        state_ = GameState::Finished;
        return bestPos;
    }

    // 切换回玩家回合
    switchPlayer();

    // 检查游戏是否结束（棋盘满了）
    if (board_.isFull()) {
        result_ = GameResult::Draw;
        state_ = GameState::Finished;
    }

    return bestPos;
}

bool GameService::undo() {
    // 至少要有一手棋才能悔棋
    if (board_.moveCount() < 1) {
        return false;
    }

    // 获取最后落子位置并移除
    const Position& lastPos = board_.lastMove();
    if (lastPos.isValid()) {
        board_.removePiece(lastPos);
    }

    // 如果AI已经落子，也移除AI的那一步
    if (current_player_ == Piece::Black && board_.moveCount() >= 1) {
        const Position& aiPos = board_.lastMove();
        if (aiPos.isValid()) {
            board_.removePiece(aiPos);
        }
    }

    // 重置游戏状态
    state_ = GameState::Playing;
    result_ = GameResult::None;
    current_player_ = Piece::Black;  // 黑方先行

    return true;
}

bool GameService::checkWin(Piece piece) {
    return win_detector_.checkWin(board_, piece);
}

void GameService::switchPlayer() {
    current_player_ = (current_player_ == Piece::Black) ? Piece::White : Piece::Black;
}

void GameService::checkGameOver() {
    // 检查是否已有结果
    if (result_ != GameResult::None) {
        return;
    }

    // 检查白方是否获胜
    if (win_detector_.checkWin(board_, Piece::White)) {
        result_ = GameResult::White;
        state_ = GameState::Finished;
        return;
    }

    // 检查黑方是否获胜
    if (win_detector_.checkWin(board_, Piece::Black)) {
        result_ = GameResult::Black;
        state_ = GameState::Finished;
        return;
    }

    // 检查是否平局（棋盘满了）
    if (board_.isFull()) {
        result_ = GameResult::Draw;
        state_ = GameState::Finished;
    }
}

} // namespace game_core
