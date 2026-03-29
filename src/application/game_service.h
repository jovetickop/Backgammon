#pragma once

#include "../domain/domain.h"

// 游戏流程应用服务：协调领域对象完成用例。
namespace game_core {

// 游戏状态
enum class GameState {
    Idle,       // 等待开始
    Playing,    // 对局进行中
    Finished    // 对局结束
};

// 游戏流程服务
class GameService {
public:
    // 构造函数
    GameService();

    // 开始新游戏
    void startNewGame();

    // 玩家落子
    // 返回值：是否成功落子
    bool playerMove(const Position& pos);

    // AI落子
    // 返回值：AI的落子位置
    Position aiMove();

    // 悔棋
    // 返回值：是否成功悔棋
    bool undo();

    // 获取当前游戏状态
    GameState state() const { return state_; }

    // 获取游戏结果
    GameResult result() const { return result_; }

    // 获取当前执子方
    Piece currentPlayer() const { return current_player_; }

    // 获取棋盘引用
    const GameBoard& board() const { return board_; }

    // 设置AI搜索深度
    void setAIDepth(int depth) { ai_engine_.setSearchDepth(depth); }

    // 获取AI搜索深度
    int aiDepth() const { return ai_engine_.searchDepth(); }

    // 检查指定玩家是否已获胜
    bool checkWin(Piece piece);

private:
    // 切换玩家
    void switchPlayer();

    // 检查游戏是否结束
    void checkGameOver();

    // 领域对象
    GameBoard board_;              // 棋盘聚合根
    WinDetector win_detector_;     // 胜负检测
    BoardEvaluator evaluator_;      // 局面评估
    AIEngine ai_engine_;           // AI引擎

    // 应用状态
    GameState state_;               // 游戏状态
    Piece current_player_;         // 当前执子方
    GameResult result_;            // 游戏结果
};

} // namespace game_core
