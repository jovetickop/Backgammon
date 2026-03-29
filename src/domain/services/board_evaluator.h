#pragma once

#include "../aggregates/game_board.h"
#include "../values/piece.h"
#include <vector>
#include <optional>

// 局面评估领域服务：评估当前局面分值。
namespace game_core {

// 棋型评分枚举
enum class ScoreType {
    OpenOne = 10,      // 活一
    OpenTwo = 100,     // 活二
    OpenThree = 10000, // 活三
    OpenFour = 1000000,// 活四
    Five = 10000000,   // 五连

    CloseOne = 1,      // 冲一
    CloseTwo = 10,     // 冲二
    CloseThree = 1000, // 冲三
    CloseFour = 100000 // 冲四
};

// 候选落子点
struct CandidateMove {
    Position position;
    int score;

    CandidateMove() : position(), score(0) {}
    CandidateMove(const Position& pos, int s) : position(pos), score(s) {}

    // 用于排序：分数高的排前面
    bool operator<(const CandidateMove& other) const {
        return score < other.score;
    }
};

// 局面评估服务
class BoardEvaluator {
public:
    // 评估局面：返回白方分 - 黑方分
    // 正值表示白方有利，负值表示黑方有利
    int evaluate(const GameBoard& board);

    // 快速评估单步落子后对指定棋子的增益分值
    int evaluateMove(const GameBoard& board, const Position& pos, Piece piece);

    // 生成候选落子点（靠近已有棋子的位置），按评估分降序排列
    std::vector<CandidateMove> generateCandidates(const GameBoard& board, Piece piece);

    // 检测指定棋子是否有"一步即可五连"的棋型（活四或冲四）
    bool hasWinningMove(const GameBoard& board, Piece piece);

    // 检测指定棋子是否有活三
    // 返回活三的数量
    int countOpenThrees(const GameBoard& board, Piece piece);

private:
    // 将棋盘按横、竖、两条斜线方向切分为一组一维线段
    std::vector<std::vector<Piece>> cutBoard(const GameBoard& board);

    // 统计指定棋子的总评分
    int score(const GameBoard& board, Piece piece, std::vector<std::vector<Piece>>& vecSum);

    // 各类棋型检测
    bool isOpenOne(const std::vector<Piece>& line, int index, Piece piece);
    bool isOpenTwo(const std::vector<Piece>& line, int index, Piece piece);
    bool isOpenThree(const std::vector<Piece>& line, int index, Piece piece);
    bool isOpenFour(const std::vector<Piece>& line, int index, Piece piece);
    bool isFive(const std::vector<Piece>& line, int index, Piece piece);
    bool isCloseOne(const std::vector<Piece>& line, int index, Piece piece);
    bool isCloseTwo(const std::vector<Piece>& line, int index, Piece piece);
    bool isCloseThree(const std::vector<Piece>& line, int index, Piece piece);
    bool isCloseFour(const std::vector<Piece>& line, int index, Piece piece);
};

} // namespace game_core
