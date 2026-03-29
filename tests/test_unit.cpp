// 单元测试 - 使用 GoogleTest 框架
#include <gtest/gtest.h>
#include <QCoreApplication>
#include "types.h"
#include "judgeWinner.h"
#include "Evaluation.h"
#include "playerstatsstore.h"

// ==================== types.h 测试 ====================

TEST(TypesTest, PieceEnumValues)
{
    EXPECT_EQ(static_cast<int>(NONE), 0);
    EXPECT_EQ(static_cast<int>(WHITE), 1);
    EXPECT_EQ(static_cast<int>(BLACK), 2);
}

TEST(TypesTest, ScoreOrdering_OpenSeries)
{
    EXPECT_LT(OPEN_ONE, OPEN_TWO);
    EXPECT_LT(OPEN_TWO, OPEN_THREE);
    EXPECT_LT(OPEN_THREE, OPEN_FOUR);
    EXPECT_LT(OPEN_FOUR, FIVE);
}

TEST(TypesTest, ScoreOrdering_CloseSeries)
{
    EXPECT_LT(CLOSE_ONE, CLOSE_TWO);
    EXPECT_LT(CLOSE_TWO, CLOSE_THREE);
    EXPECT_LT(CLOSE_THREE, CLOSE_FOUR);
}

TEST(TypesTest, OpenHigherThanClose)
{
    // 活棋型分值应高于对应的死棋型
    EXPECT_GT(OPEN_FOUR, CLOSE_FOUR);
    EXPECT_GT(OPEN_THREE, CLOSE_THREE);
    EXPECT_GT(OPEN_TWO, CLOSE_TWO);
}

TEST(TypesTest, FiveIsHighest)
{
    EXPECT_GT(FIVE, OPEN_FOUR);
    EXPECT_GT(FIVE, CLOSE_FOUR);
}

// ==================== judgeWinner 测试 ====================

class JudgeWinnerTest : public ::testing::Test
{
protected:
    judgeWinner judge;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board)); // NONE = 0
    }
};

TEST_F(JudgeWinnerTest, EmptyBoard_NoWinner)
{
    EXPECT_FALSE(judge.IsWon(BLACK, board));
    EXPECT_FALSE(judge.IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, Black_HorizontalFive_Wins)
{
    // 第7行 cols 0-4 五连
    for (int i = 0; i < 5; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
    EXPECT_FALSE(judge.IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, White_VerticalFive_Wins)
{
    // 第7列 rows 0-4 五连
    for (int i = 0; i < 5; ++i) board[i][7] = WHITE;
    EXPECT_TRUE(judge.IsWon(WHITE, board));
    EXPECT_FALSE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, Black_DiagonalRightDown_Wins)
{
    // 主对角线方向
    for (int i = 0; i < 5; ++i) board[i][i] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, Black_DiagonalRightUp_Wins)
{
    // 反对角线方向
    for (int i = 0; i < 5; ++i) board[i][14 - i] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FourInRow_NoWinner)
{
    for (int i = 0; i < 4; ++i) board[7][i] = BLACK;
    EXPECT_FALSE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, SixInRow_StillWins)
{
    // 六子含五连子集，仍算获胜
    for (int i = 0; i < 6; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_BottomEdge)
{
    // 最后一行边缘
    for (int j = 10; j <= 14; ++j) board[14][j] = WHITE;
    EXPECT_TRUE(judge.IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_LeftEdge)
{
    // 第一列边缘
    for (int i = 0; i < 5; ++i) board[i][0] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_TopRightCorner)
{
    // 右上角附近主对角线
    for (int i = 0; i < 5; ++i) board[i][10 + i] = WHITE;
    EXPECT_TRUE(judge.IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, Interrupted_NoWinner)
{
    // 中间被对方棋子打断
    for (int i = 0; i < 4; ++i) board[7][i] = BLACK;
    board[7][2] = WHITE; // 打断
    EXPECT_FALSE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, OpponentWins_SelfDoesNot)
{
    // 白棋五连，黑棋不算赢
    for (int i = 0; i < 5; ++i) board[7][i] = WHITE;
    EXPECT_TRUE(judge.IsWon(WHITE, board));
    EXPECT_FALSE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, DiagonalRightDown_BottomRight)
{
    // 右下角主对角
    for (int i = 0; i < 5; ++i) board[10 + i][10 + i] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, DiagonalRightUp_BottomLeft)
{
    // 左下角反对角
    for (int i = 0; i < 5; ++i) board[14 - i][i] = WHITE;
    EXPECT_TRUE(judge.IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, ExactlyFive_NotFour)
{
    // 恰好五子：确认检测精度
    board[5][5] = BLACK;
    board[5][6] = BLACK;
    board[5][7] = BLACK;
    board[5][8] = BLACK;
    board[5][9] = BLACK;
    EXPECT_TRUE(judge.IsWon(BLACK, board));
}

// ==================== Evaluation 测试 ====================

class EvaluationTest : public ::testing::Test
{
protected:
    Evaluation eval;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board));
    }
};

TEST_F(EvaluationTest, EmptyBoard_NearZero)
{
    int score = eval.EvaluateBoard(board);
    EXPECT_GE(score, -100);
    EXPECT_LE(score, 100);
}

TEST_F(EvaluationTest, BlackFive_MaxNegativeScore)
{
    // 黑棋五连，得分应接近 -FIVE
    for (int i = 0; i < 5; ++i) board[7][i] = BLACK;
    int score = eval.EvaluateBoard(board);
    EXPECT_LE(score, -FIVE + 100);
}

TEST_F(EvaluationTest, WhiteFive_MaxPositiveScore)
{
    // 白棋五连，得分应接近 +FIVE
    for (int i = 0; i < 5; ++i) board[7][i] = WHITE;
    int score = eval.EvaluateBoard(board);
    EXPECT_GE(score, FIVE - 100);
}

TEST_F(EvaluationTest, BlackOpenFour_NegativeScore)
{
    // 黑棋活四（两端开放），整体得分应为负
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = eval.EvaluateBoard(board);
    EXPECT_LT(score, 0);
}

TEST_F(EvaluationTest, WhiteOpenFour_PositiveScore)
{
    // 白棋活四，整体得分应为正
    for (int i = 3; i <= 6; ++i) board[7][i] = WHITE;
    int score = eval.EvaluateBoard(board);
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, BlackOpenThree_NegativeScore)
{
    // 黑棋活三，得分应为负
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int score = eval.EvaluateBoard(board);
    EXPECT_LT(score, 0);
}

TEST_F(EvaluationTest, WhiteOpenThree_PositiveScore)
{
    // 白棋活三，得分应为正
    for (int i = 4; i <= 6; ++i) board[7][i] = WHITE;
    int score = eval.EvaluateBoard(board);
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, SymmetricBoard_NearZero)
{
    // 对称落子，整体趋近平衡
    board[7][5] = BLACK;
    board[7][9] = WHITE;
    int score = eval.EvaluateBoard(board);
    // 单子对称，分差不应过大
    EXPECT_GT(score, -OPEN_THREE);
    EXPECT_LT(score, OPEN_THREE);
}

TEST_F(EvaluationTest, WhiteFive_Beats_BlackOpenFour)
{
    // 白五连 vs 黑活四，白方优势（正值）
    for (int i = 0; i < 5; ++i) board[3][i] = WHITE;
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = eval.EvaluateBoard(board);
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, VerticalFive_AlsoDetected)
{
    // 纵向五连同样得到最高分
    for (int i = 0; i < 5; ++i) board[i][7] = BLACK;
    int score = eval.EvaluateBoard(board);
    EXPECT_LE(score, -FIVE + 100);
}

// ==================== PlayerStatsStore 测试 ====================

class PlayerStatsStoreTest : public ::testing::Test
{
protected:
    PlayerStatsStore store;
};

TEST_F(PlayerStatsStoreTest, NewUser_DefaultRecord)
{
    PlayerRecord rec = store.RecordForUser("Alice");
    EXPECT_EQ(rec.displayName, QString("Alice"));
    EXPECT_EQ(rec.wins, 0);
    EXPECT_EQ(rec.losses, 0);
    EXPECT_EQ(rec.preferredStarter, QString("ai"));
    EXPECT_TRUE(rec.games.isEmpty());
}

TEST_F(PlayerStatsStoreTest, SaveAndRetrieve_RoundTrip)
{
    PlayerRecord rec;
    rec.displayName = "Bob";
    rec.preferredStarter = "player";
    rec.wins = 3;
    rec.losses = 1;
    store.SaveRecord(rec);

    PlayerRecord retrieved = store.RecordForUser("Bob");
    EXPECT_EQ(retrieved.displayName, QString("Bob"));
    EXPECT_EQ(retrieved.preferredStarter, QString("player"));
}

TEST_F(PlayerStatsStoreTest, NormalizeUserName_CaseInsensitive)
{
    // "Alice" 和 "alice" 应指向同一条记录
    PlayerRecord rec;
    rec.displayName = "Alice";
    rec.preferredStarter = "player";
    store.SaveRecord(rec);

    PlayerRecord retrieved = store.RecordForUser("ALICE");
    EXPECT_EQ(retrieved.preferredStarter, QString("player"));
}

TEST_F(PlayerStatsStoreTest, NormalizeUserName_TrimsSpaces)
{
    PlayerRecord rec;
    rec.displayName = "  Charlie  ";
    rec.preferredStarter = "player";
    store.SaveRecord(rec);

    // 带空格的用户名应能被找到
    PlayerRecord retrieved = store.RecordForUser("charlie");
    EXPECT_EQ(retrieved.preferredStarter, QString("player"));
}

TEST_F(PlayerStatsStoreTest, NormalizeStarter_Player)
{
    PlayerRecord rec;
    rec.displayName = "Dave";
    rec.preferredStarter = "Player"; // 首字母大写
    store.SaveRecord(rec);

    PlayerRecord retrieved = store.RecordForUser("Dave");
    EXPECT_EQ(retrieved.preferredStarter, QString("player"));
}

TEST_F(PlayerStatsStoreTest, NormalizeStarter_UnknownBecomesAI)
{
    PlayerRecord rec;
    rec.displayName = "Eve";
    rec.preferredStarter = "human"; // 非法值
    store.SaveRecord(rec);

    PlayerRecord retrieved = store.RecordForUser("Eve");
    EXPECT_EQ(retrieved.preferredStarter, QString("ai"));
}

TEST_F(PlayerStatsStoreTest, LastUser_DefaultEmpty)
{
    EXPECT_TRUE(store.LastUser().isEmpty());
}

TEST_F(PlayerStatsStoreTest, SetLastUser_RoundTrip)
{
    store.SetLastUser("Frank");
    EXPECT_EQ(store.LastUser(), QString("Frank"));
}

TEST_F(PlayerStatsStoreTest, SetLastUser_TrimsSpaces)
{
    store.SetLastUser("  Grace  ");
    EXPECT_EQ(store.LastUser(), QString("Grace"));
}

TEST_F(PlayerStatsStoreTest, RecentUsers_InitiallyEmpty)
{
    EXPECT_TRUE(store.RecentUsers().isEmpty());
}

TEST_F(PlayerStatsStoreTest, TouchRecentUser_AddsUser)
{
    store.TouchRecentUser("Hank");
    EXPECT_TRUE(store.RecentUsers().contains("Hank"));
}

TEST_F(PlayerStatsStoreTest, TouchRecentUser_DeduplicatesCaseInsensitive)
{
    store.TouchRecentUser("Ivy");
    store.TouchRecentUser("IVY"); // 重复
    EXPECT_EQ(store.RecentUsers().size(), 1);
}

TEST_F(PlayerStatsStoreTest, TouchRecentUser_MovesToFront)
{
    store.TouchRecentUser("Jack");
    store.TouchRecentUser("Kate");
    store.TouchRecentUser("Jack"); // 再次触碰，应移到最前
    EXPECT_EQ(store.RecentUsers().first(), QString("Jack"));
}

TEST_F(PlayerStatsStoreTest, TouchRecentUser_MaxEightUsers)
{
    for (int i = 0; i < 10; ++i)
        store.TouchRecentUser(QString("User%1").arg(i));
    EXPECT_LE(store.RecentUsers().size(), 8);
}

TEST_F(PlayerStatsStoreTest, ComputeWinsAndLosses_CountsCorrectly)
{
    PlayerRecord rec;
    rec.displayName = "Leo";

    GameRecord g1; g1.playerWon = true;  g1.moveCount = 20;
    GameRecord g2; g2.playerWon = false; g2.moveCount = 18;
    GameRecord g3; g3.playerWon = true;  g3.moveCount = 22;
    rec.games = {g1, g2, g3};

    store.SaveRecord(rec);
    PlayerRecord retrieved = store.RecordForUser("Leo");
    EXPECT_EQ(retrieved.wins, 2);
    EXPECT_EQ(retrieved.losses, 1);
}

TEST_F(PlayerStatsStoreTest, EmptyUserName_NotSaved)
{
    PlayerRecord rec;
    rec.displayName = "";
    store.SaveRecord(rec); // 不应崩溃

    // 空用户名查询应返回默认空记录
    PlayerRecord retrieved = store.RecordForUser("");
    EXPECT_EQ(retrieved.wins, 0);
}

TEST_F(PlayerStatsStoreTest, MultipleUsers_Independent)
{
    PlayerRecord r1; r1.displayName = "Mike"; r1.preferredStarter = "player";
    PlayerRecord r2; r2.displayName = "Nina"; r2.preferredStarter = "ai";
    store.SaveRecord(r1);
    store.SaveRecord(r2);

    EXPECT_EQ(store.RecordForUser("Mike").preferredStarter, QString("player"));
    EXPECT_EQ(store.RecordForUser("Nina").preferredStarter, QString("ai"));
}

// ==================== HasWinningMove / CountOpenThrees / EvaluateMove 测试 ====================

class EvaluationAdvancedTest : public ::testing::Test
{
protected:
    Evaluation eval;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board));
    }
};

// ---------- HasWinningMove 测试：检测活四或冲四（一步即可五连） ----------

TEST_F(EvaluationAdvancedTest, HasWinningMove_EmptyBoard_False)
{
    // 空棋盘没有任何一方有必赢局面。
    EXPECT_FALSE(eval.HasWinningMove(board, BLACK));
    EXPECT_FALSE(eval.HasWinningMove(board, WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_HorizontalOpenFour_True)
{
    // 黑棋横向活四（7行 cols 3~6，两端均空），可一步五连。
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(eval.HasWinningMove(board, BLACK));
    EXPECT_FALSE(eval.HasWinningMove(board, WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_VerticalCloseFour_True)
{
    // 白棋纵向冲四（列7 rows 3~6，上方被堵，下方 col=7 row=7 为空）。
    for (int i = 3; i <= 6; ++i) board[i][7] = WHITE;
    board[2][7] = BLACK; // 上端封堵
    EXPECT_TRUE(eval.HasWinningMove(board, WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_DeadFour_False)
{
    // 黑棋四连但两端都被堵死，不算必赢局面。
    for (int i = 4; i <= 7; ++i) board[7][i] = BLACK;
    board[7][3] = WHITE; // 左端堵
    board[7][8] = WHITE; // 右端堵
    EXPECT_FALSE(eval.HasWinningMove(board, BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_OpenThree_False)
{
    // 活三不算必赢（需要两步才能五连），应返回 false。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    EXPECT_FALSE(eval.HasWinningMove(board, BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_DiagonalOpenFour_True)
{
    // 主对角线方向活四。
    for (int i = 5; i <= 8; ++i) board[i][i] = BLACK;
    EXPECT_TRUE(eval.HasWinningMove(board, BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_AntiDiagonalCloseFour_True)
{
    // 反对角线方向冲四（上端封堵）。
    for (int i = 3; i <= 6; ++i) board[i][14 - i] = WHITE;
    board[2][12] = BLACK; // 上端封堵
    EXPECT_TRUE(eval.HasWinningMove(board, WHITE));
}

// ---------- CountOpenThrees 测试：检测活三数量 ----------

TEST_F(EvaluationAdvancedTest, CountOpenThrees_EmptyBoard_NoThrees)
{
    // 空棋盘没有活三。
    EXPECT_EQ(eval.CountOpenThrees(board, BLACK), -1);
    EXPECT_EQ(eval.CountOpenThrees(board, WHITE), -1);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_SingleOpenThree_Positive)
{
    // 单个横向活三（7行 cols 4~6），应返回正值（至少2个关键位置）。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int count = eval.CountOpenThrees(board, BLACK);
    EXPECT_GT(count, 0);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_CloseThree_SameOrLessThanOpenThree)
{
    // 眠三（一端被封堵）的关键位置数应小于或等于活三（两端开放）。
    // 活三场景：7行 cols 4~6。
    ePiece openBoard[15][15];
    memset(openBoard, 0, sizeof(openBoard));
    for (int i = 4; i <= 6; ++i) openBoard[7][i] = BLACK;
    int openCount = eval.CountOpenThrees(openBoard, BLACK);

    // 眠三场景：7行 cols 4~6，左端 col 3 被白棋封堵。
    ePiece closeBoard[15][15];
    memset(closeBoard, 0, sizeof(closeBoard));
    for (int i = 4; i <= 6; ++i) closeBoard[7][i] = BLACK;
    closeBoard[7][3] = WHITE;
    int closeCount = eval.CountOpenThrees(closeBoard, BLACK);

    // 眠三的关键位置数不应超过活三。
    EXPECT_LE(closeCount, openCount);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_DeadThree_NotCounted)
{
    // 死三（两端被封堵）不应算作活三。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    board[7][3] = WHITE; // 左端封堵
    board[7][7] = WHITE; // 右端封堵
    int count = eval.CountOpenThrees(board, BLACK);
    EXPECT_EQ(count, -1);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_OpponentHasNoThrees)
{
    // 黑棋有活三，白棋不应有活三。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int whiteCount = eval.CountOpenThrees(board, WHITE);
    EXPECT_EQ(whiteCount, -1);
}

// ---------- EvaluateMove 测试：单步落子评估 ----------

TEST_F(EvaluationAdvancedTest, EvaluateMove_EmptyBoard_NonZero)
{
    // 在空棋盘上落子，评估分应非零（产生活一分值）。
    int score = eval.EvaluateMove(board, 7, 7, BLACK);
    EXPECT_NE(score, 0);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_ScoreRestoresBoard)
{
    // 调用 EvaluateMove 后棋盘应恢复原状（不留下临时落子）。
    int beforeScore = eval.EvaluateBoard(board);
    eval.EvaluateMove(board, 7, 7, WHITE);
    int afterScore = eval.EvaluateBoard(board);
    EXPECT_EQ(beforeScore, afterScore);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_WinningMove_HighScore)
{
    // 落子直接形成五连，分值应极高（接近 FIVE）。
    // 先放置四连黑子（7行 cols 3~6），然后在 col 7 落子。
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = eval.EvaluateMove(board, 7, 7, BLACK);
    // 落子后形成五连，EvaluateBoard 返回值包含 -FIVE，分值应非常低（大负数）。
    EXPECT_LT(score, -FIVE / 2);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_OpenFourHigherThanOpenThree)
{
    // 落子形成活四的得分应远高于落子形成活三的得分。
    // 活四场景：7行已有3子（cols 4~6），落子 col 3 形成活四。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int openFourScore = eval.EvaluateMove(board, 7, 3, BLACK);

    // 活三场景：7行已有2子（cols 5~6），落子 col 4 形成活三。
    memset(board, 0, sizeof(board));
    for (int i = 5; i <= 6; ++i) board[7][i] = BLACK;
    int openThreeScore = eval.EvaluateMove(board, 7, 4, BLACK);

    EXPECT_LT(openFourScore, openThreeScore); // 活四（更利于黑棋）分值更低
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_DefenseValue_Positive)
{
    // 阻断对手棋型有价值：白棋已有活三，黑棋在端点落子可破坏白棋的扩展空间。
    for (int i = 4; i <= 6; ++i) board[7][i] = WHITE;
    // 黑棋在 col 3 落子（白活三的左端点），评估黑棋视角：减少白方分即有利于黑方。
    int blackDefenseScore = eval.EvaluateMove(board, 7, 3, BLACK);
    // EvaluateMove 返回 EvaluateBoard 值，阻断白棋活三应使白方分降低（分值趋负）。
    // 但由于该位置本身也有黑棋价值，更合理的断言是阻断后分值低于不阻断时。
    int baseScore = eval.EvaluateBoard(board); // 未落子时的棋盘分值
    // 落子后白方分值应减小（黑棋扩张+白棋被阻），因此 EvaluateMove 结果 < baseScore。
    EXPECT_LT(blackDefenseScore, baseScore);
}

// ==================== 主函数（提供 QCoreApplication）====================

int main(int argc, char **argv)
{
    // 部分 Qt 操作（如 QStandardPaths）需要 QCoreApplication
    QCoreApplication app(argc, argv);
    app.setApplicationName("BackgammonTest");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
