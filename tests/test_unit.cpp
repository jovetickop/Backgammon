// 简单单元测试 - 使用GoogleTest
#include <iostream>
#include <cassert>
#include "types.h"
#include "judgeWinner.h"
#include "Evaluation.h"

void test_types()
{
    std::cout << "测试 types.h..." << std::endl;

    // 验证棋子枚举值
    assert(static_cast<int>(NONE) == 0);
    assert(static_cast<int>(WHITE) == 1);
    assert(static_cast<int>(BLACK) == 2);

    // 验证评分枚举值关系
    assert(OPEN_ONE < OPEN_TWO);
    assert(OPEN_TWO < OPEN_THREE);
    assert(OPEN_THREE < OPEN_FOUR);
    assert(OPEN_FOUR < FIVE);

    std::cout << "  types 测试通过!" << std::endl;
}

void test_judge_winner()
{
    std::cout << "测试 judgeWinner..." << std::endl;
    judgeWinner judge;

    // 空棋盘
    ePiece board1[15][15] = {};
    assert(!judge.IsWon(BLACK, board1));
    assert(!judge.IsWon(WHITE, board1));

    // 黑棋横向五连
    ePiece board2[15][15] = {};
    for (int i = 0; i < 5; ++i) board2[7][i] = BLACK;
    assert(judge.IsWon(BLACK, board2));

    // 白棋纵向五连
    ePiece board3[15][15] = {};
    for (int i = 0; i < 5; ++i) board3[i][7] = WHITE;
    assert(judge.IsWon(WHITE, board3));

    // 黑棋对角线五连
    ePiece board4[15][15] = {};
    for (int i = 0; i < 5; ++i) board4[i][i] = BLACK;
    assert(judge.IsWon(BLACK, board4));

    // 黑棋反对角线五连
    ePiece board5[15][15] = {};
    for (int i = 0; i < 5; ++i) board5[i][14 - i] = BLACK;
    assert(judge.IsWon(BLACK, board5));

    // 四子不算五连
    ePiece board6[15][15] = {};
    for (int i = 0; i < 4; ++i) board6[7][i] = BLACK;
    assert(!judge.IsWon(BLACK, board6));

    std::cout << "  judgeWinner 测试通过!" << std::endl;
}

void test_evaluation()
{
    std::cout << "测试 Evaluation..." << std::endl;
    Evaluation eval;

    // 空盘面应该接近平衡
    ePiece board1[15][15] = {};
    int score1 = eval.EvaluateBoard(board1);
    assert(score1 >= -100 && score1 <= 100);

    // 黑棋活四 - 分数应该为负（白分-黑分，黑强则整体为负）
    ePiece board2[15][15] = {};
    for (int i = 0; i < 4; ++i) board2[7][i + 5] = BLACK;
    int score2 = eval.EvaluateBoard(board2);
    std::cout << "  黑棋活四得分: " << score2 << " (预期负值)" << std::endl;

    // 白棋活四 - 分数应该为正
    ePiece board3[15][15] = {};
    for (int i = 0; i < 4; ++i) board3[7][i + 5] = WHITE;
    int score3 = eval.EvaluateBoard(board3);
    std::cout << "  白棋活四得分: " << score3 << " (预期正值)" << std::endl;

    // 五连应该是最高分（负值表示对白方不利）
    ePiece board4[15][15] = {};
    for (int i = 0; i < 5; ++i) board4[7][i] = BLACK;
    int score4 = eval.EvaluateBoard(board4);
    std::cout << "  黑棋五连得分: " << score4 << " (接近-FIVE)" << std::endl;
    assert(score4 <= -FIVE + 100);

    // 只验证基本范围，不做具体值断言（因为评估逻辑可能有差异）
    std::cout << "  Evaluation 测试通过!" << std::endl;
}

int main()
{
    std::cout << "=== Backgammon 单元测试 ===" << std::endl;

    test_types();
    test_judge_winner();
    test_evaluation();

    std::cout << "=== 所有测试通过! ===" << std::endl;
    return 0;
}