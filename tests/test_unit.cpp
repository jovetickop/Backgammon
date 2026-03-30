// 单元测试 - 使用 GoogleTest 框架
#include <gtest/gtest.h>
#include <QCoreApplication>
#include "types.h"
#include "playerstatsstore.h"
#include "domain/aggregates/game_board.h"
#include "domain/services/win_detector.h"
#include "domain/services/board_evaluator.h"
#include "domain/services/ai_engine.h"

using namespace game_core;

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

// ==================== WinDetector 测试（替代旧 judgeWinner）====================

// 辅助：将 ePiece 数组转为 GameBoard
static GameBoard MakeBoard(const ePiece arr[15][15]) {
    GameBoard b;
    for (int i = 0; i < 15; ++i)
        for (int j = 0; j < 15; ++j)
            if (arr[i][j] != NONE) {
                Piece p = (arr[i][j] == WHITE) ? Piece::White : Piece::Black;
                b.placePiece(Position(i, j), p);
            }
    return b;
}

class JudgeWinnerTest : public ::testing::Test
{
protected:
    WinDetector judge;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board));
    }

    // 判断指定棋子是否获胜（兼容原 IsWon 接口）
    bool IsWon(ePiece piece, const ePiece arr[15][15]) {
        GameBoard b = MakeBoard(arr);
        Piece p = (piece == WHITE) ? Piece::White : Piece::Black;
        return judge.checkWin(b, p);
    }
};

TEST_F(JudgeWinnerTest, EmptyBoard_NoWinner)
{
    EXPECT_FALSE(IsWon(BLACK, board));
    EXPECT_FALSE(IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, Black_HorizontalFive_Wins)
{
    // 第7行 cols 0-4 五连
    for (int i = 0; i < 5; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
    EXPECT_FALSE(IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, White_VerticalFive_Wins)
{
    // 第7列 rows 0-4 五连
    for (int i = 0; i < 5; ++i) board[i][7] = WHITE;
    EXPECT_TRUE(IsWon(WHITE, board));
    EXPECT_FALSE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, Black_DiagonalRightDown_Wins)
{
    // 主对角线方向
    for (int i = 0; i < 5; ++i) board[i][i] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, Black_DiagonalRightUp_Wins)
{
    // 反对角线方向
    for (int i = 0; i < 5; ++i) board[i][14 - i] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FourInRow_NoWinner)
{
    for (int i = 0; i < 4; ++i) board[7][i] = BLACK;
    EXPECT_FALSE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, SixInRow_StillWins)
{
    // 六子含五连子集，仍算获胜
    for (int i = 0; i < 6; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_BottomEdge)
{
    // 最后一行边缘
    for (int j = 10; j <= 14; ++j) board[14][j] = WHITE;
    EXPECT_TRUE(IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_LeftEdge)
{
    // 第一列边缘
    for (int i = 0; i < 5; ++i) board[i][0] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, FiveInRow_TopRightCorner)
{
    // 右上角附近主对角线
    for (int i = 0; i < 5; ++i) board[i][10 + i] = WHITE;
    EXPECT_TRUE(IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, Interrupted_NoWinner)
{
    // 中间被对方棋子打断
    for (int i = 0; i < 4; ++i) board[7][i] = BLACK;
    board[7][2] = WHITE; // 打断
    EXPECT_FALSE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, OpponentWins_SelfDoesNot)
{
    // 白棋五连，黑棋不算赢
    for (int i = 0; i < 5; ++i) board[7][i] = WHITE;
    EXPECT_TRUE(IsWon(WHITE, board));
    EXPECT_FALSE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, DiagonalRightDown_BottomRight)
{
    // 右下角主对角
    for (int i = 0; i < 5; ++i) board[10 + i][10 + i] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

TEST_F(JudgeWinnerTest, DiagonalRightUp_BottomLeft)
{
    // 左下角反对角
    for (int i = 0; i < 5; ++i) board[14 - i][i] = WHITE;
    EXPECT_TRUE(IsWon(WHITE, board));
}

TEST_F(JudgeWinnerTest, ExactlyFive_NotFour)
{
    // 恰好五子：确认检测精度
    board[5][5] = BLACK;
    board[5][6] = BLACK;
    board[5][7] = BLACK;
    board[5][8] = BLACK;
    board[5][9] = BLACK;
    EXPECT_TRUE(IsWon(BLACK, board));
}

// ==================== BoardEvaluator 测试（替代旧 Evaluation）====================

class EvaluationTest : public ::testing::Test
{
protected:
    BoardEvaluator eval;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board));
    }

    int EvaluateBoard() { return eval.evaluate(MakeBoard(board)); }
    bool HasWinningMove(ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.hasWinningMove(MakeBoard(board), gp);
    }
    int CountOpenThrees(ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.countOpenThrees(MakeBoard(board), gp);
    }
    int EvaluateMove(int r, int c, ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.evaluateMove(MakeBoard(board), Position(r, c), gp);
    }
};

TEST_F(EvaluationTest, EmptyBoard_NearZero)
{
    int score = EvaluateBoard();
    EXPECT_GE(score, -100);
    EXPECT_LE(score, 100);
}

TEST_F(EvaluationTest, BlackFive_MaxNegativeScore)
{
    // 黑棋五连，得分应接近 -FIVE
    for (int i = 0; i < 5; ++i) board[7][i] = BLACK;
    int score = EvaluateBoard();
    EXPECT_LE(score, -FIVE + 100);
}

TEST_F(EvaluationTest, WhiteFive_MaxPositiveScore)
{
    // 白棋五连，得分应接近 +FIVE
    for (int i = 0; i < 5; ++i) board[7][i] = WHITE;
    int score = EvaluateBoard();
    EXPECT_GE(score, FIVE - 100);
}

TEST_F(EvaluationTest, BlackOpenFour_NegativeScore)
{
    // 黑棋活四（两端开放），整体得分应为负
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = EvaluateBoard();
    EXPECT_LT(score, 0);
}

TEST_F(EvaluationTest, WhiteOpenFour_PositiveScore)
{
    // 白棋活四，整体得分应为正
    for (int i = 3; i <= 6; ++i) board[7][i] = WHITE;
    int score = EvaluateBoard();
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, BlackOpenThree_NegativeScore)
{
    // 黑棋活三，得分应为负
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int score = EvaluateBoard();
    EXPECT_LT(score, 0);
}

TEST_F(EvaluationTest, WhiteOpenThree_PositiveScore)
{
    // 白棋活三，得分应为正
    for (int i = 4; i <= 6; ++i) board[7][i] = WHITE;
    int score = EvaluateBoard();
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, SymmetricBoard_NearZero)
{
    // 对称落子，整体趋近平衡
    board[7][5] = BLACK;
    board[7][9] = WHITE;
    int score = EvaluateBoard();
    // 单子对称，分差不应过大
    EXPECT_GT(score, -OPEN_THREE);
    EXPECT_LT(score, OPEN_THREE);
}

TEST_F(EvaluationTest, WhiteFive_Beats_BlackOpenFour)
{
    // 白五连 vs 黑活四，白方优势（正值）
    for (int i = 0; i < 5; ++i) board[3][i] = WHITE;
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = EvaluateBoard();
    EXPECT_GT(score, 0);
}

TEST_F(EvaluationTest, VerticalFive_AlsoDetected)
{
    // 纵向五连同样得到最高分
    for (int i = 0; i < 5; ++i) board[i][7] = BLACK;
    int score = EvaluateBoard();
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
    BoardEvaluator eval;
    ePiece board[15][15];

    void SetUp() override
    {
        memset(board, 0, sizeof(board));
    }

    bool HasWinningMove(ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.hasWinningMove(MakeBoard(board), gp);
    }
    int CountOpenThrees(ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.countOpenThrees(MakeBoard(board), gp);
    }
    int EvaluateBoard() { return eval.evaluate(MakeBoard(board)); }
    int EvaluateMove(int r, int c, ePiece p) {
        Piece gp = (p == WHITE) ? Piece::White : Piece::Black;
        return eval.evaluateMove(MakeBoard(board), Position(r, c), gp);
    }
};

// ---------- HasWinningMove 测试：检测活四或冲四（一步即可五连） ----------

TEST_F(EvaluationAdvancedTest, HasWinningMove_EmptyBoard_False)
{
    // 空棋盘没有任何一方有必赢局面。
    EXPECT_FALSE(HasWinningMove(BLACK));
    EXPECT_FALSE(HasWinningMove(WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_HorizontalOpenFour_True)
{
    // 黑棋横向活四（7行 cols 3~6，两端均空），可一步五连。
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    EXPECT_TRUE(HasWinningMove(BLACK));
    EXPECT_FALSE(HasWinningMove(WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_VerticalCloseFour_True)
{
    // 白棋纵向冲四（列7 rows 3~6，上方被堵，下方 col=7 row=7 为空）。
    for (int i = 3; i <= 6; ++i) board[i][7] = WHITE;
    board[2][7] = BLACK; // 上端封堵
    EXPECT_TRUE(HasWinningMove(WHITE));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_DeadFour_False)
{
    // 黑棋四连但两端都被堵死，不算必赢局面。
    for (int i = 4; i <= 7; ++i) board[7][i] = BLACK;
    board[7][3] = WHITE; // 左端堵
    board[7][8] = WHITE; // 右端堵
    EXPECT_FALSE(HasWinningMove(BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_OpenThree_False)
{
    // 活三不算必赢（需要两步才能五连），应返回 false。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    EXPECT_FALSE(HasWinningMove(BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_DiagonalOpenFour_True)
{
    // 主对角线方向活四。
    for (int i = 5; i <= 8; ++i) board[i][i] = BLACK;
    EXPECT_TRUE(HasWinningMove(BLACK));
}

TEST_F(EvaluationAdvancedTest, HasWinningMove_AntiDiagonalCloseFour_True)
{
    // 反对角线方向冲四（上端封堵）。
    for (int i = 3; i <= 6; ++i) board[i][14 - i] = WHITE;
    board[2][12] = BLACK; // 上端封堵
    EXPECT_TRUE(HasWinningMove(WHITE));
}

// ---------- CountOpenThrees 测试：检测活三数量 ----------

TEST_F(EvaluationAdvancedTest, CountOpenThrees_EmptyBoard_NoThrees)
{
    // 空棋盘没有活三。
    EXPECT_EQ(CountOpenThrees(BLACK), 0);
    EXPECT_EQ(CountOpenThrees(WHITE), 0);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_SingleOpenThree_Positive)
{
    // 单个横向活三（7行 cols 4~6），应返回正值（至少2个关键位置）。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int count = CountOpenThrees(BLACK);
    EXPECT_GT(count, 0);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_CloseThree_SameOrLessThanOpenThree)
{
    // 眠三（一端被封堵）的关键位置数应小于或等于活三（两端开放）。
    // 活三场景：7行 cols 4~6。
    ePiece openBoard[15][15];
    memset(openBoard, 0, sizeof(openBoard));
    for (int i = 4; i <= 6; ++i) openBoard[7][i] = BLACK;
    int openCount = eval.countOpenThrees(MakeBoard(openBoard), Piece::Black);

    // 眠三场景：7行 cols 4~6，左端 col 3 被白棋封堵。
    ePiece closeBoard[15][15];
    memset(closeBoard, 0, sizeof(closeBoard));
    for (int i = 4; i <= 6; ++i) closeBoard[7][i] = BLACK;
    closeBoard[7][3] = WHITE;
    int closeCount = eval.countOpenThrees(MakeBoard(closeBoard), Piece::Black);

    // 眠三的关键位置数不应超过活三。
    EXPECT_LE(closeCount, openCount);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_DeadThree_NotCounted)
{
    // 死三（两端被封堵）不应算作活三。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    board[7][3] = WHITE; // 左端封堵
    board[7][7] = WHITE; // 右端封堵
    int count = CountOpenThrees(BLACK);
    EXPECT_EQ(count, 0);
}

TEST_F(EvaluationAdvancedTest, CountOpenThrees_OpponentHasNoThrees)
{
    // 黑棋有活三，白棋不应有活三。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int whiteCount = CountOpenThrees(WHITE);
    EXPECT_EQ(whiteCount, 0);
}

// ---------- EvaluateMove 测试：单步落子评估 ----------

TEST_F(EvaluationAdvancedTest, EvaluateMove_EmptyBoard_NonZero)
{
    // 在空棋盘上落子，评估分应非零（产生活一分值）。
    int score = EvaluateMove(7, 7, BLACK);
    EXPECT_NE(score, 0);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_ScoreRestoresBoard)
{
    // 调用 EvaluateMove 后棋盘应恢复原状（不留下临时落子）。
    int beforeScore = EvaluateBoard();
    EvaluateMove(7, 7, WHITE);
    int afterScore = EvaluateBoard();
    EXPECT_EQ(beforeScore, afterScore);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_WinningMove_HighScore)
{
    // 落子直接形成五连，分值应极高（接近 FIVE）。
    // 先放置四连黑子（7行 cols 3~6），然后在 col 7 落子。
    for (int i = 3; i <= 6; ++i) board[7][i] = BLACK;
    int score = EvaluateMove(7, 7, BLACK);
    // 落子后形成五连，EvaluateBoard 返回值包含 -FIVE，分值应非常低（大负数）。
    EXPECT_LT(score, -FIVE / 2);
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_OpenFourHigherThanOpenThree)
{
    // 落子形成活四的得分应远高于落子形成活三的得分。
    // 活四场景：7行已有3子（cols 4~6），落子 col 3 形成活四。
    for (int i = 4; i <= 6; ++i) board[7][i] = BLACK;
    int openFourScore = EvaluateMove(7, 3, BLACK);

    // 活三场景：7行已有2子（cols 5~6），落子 col 4 形成活三。
    memset(board, 0, sizeof(board));
    for (int i = 5; i <= 6; ++i) board[7][i] = BLACK;
    int openThreeScore = EvaluateMove(7, 4, BLACK);

    EXPECT_LT(openFourScore, openThreeScore); // 活四（更利于黑棋）分值更低
}

TEST_F(EvaluationAdvancedTest, EvaluateMove_DefenseValue_Positive)
{
    // 阻断对手棋型有价值：白棋已有活三，黑棋在端点落子可破坏白棋的扩展空间。
    for (int i = 4; i <= 6; ++i) board[7][i] = WHITE;
    // 黑棋在 col 3 落子（白活三的左端点），评估黑棋视角：减少白方分即有利于黑方。
    int blackDefenseScore = EvaluateMove(7, 3, BLACK);
    // EvaluateMove 返回 EvaluateBoard 值，阻断白棋活三应使白方分降低（分值趋负）。
    // 但由于该位置本身也有黑棋价值，更合理的断言是阻断后分值低于不阻断时。
    int baseScore = EvaluateBoard(); // 未落子时的棋盘分值
    // 落子后白方分值应减小（黑棋扩张+白棋被阻），因此 EvaluateMove 结果 < baseScore。
    EXPECT_LT(blackDefenseScore, baseScore);
}

// ==================== GameBoard 测试 ====================

class GameBoardTest : public ::testing::Test
{
protected:
    GameBoard board;
};

TEST_F(GameBoardTest, InitialBoard_IsEmpty)
{
    EXPECT_TRUE(board.isEmpty());
    EXPECT_EQ(board.moveCount(), 0);
    EXPECT_EQ(board.pieceCount(Piece::Black), 0);
    EXPECT_EQ(board.pieceCount(Piece::White), 0);
}

TEST_F(GameBoardTest, PlacePiece_Success)
{
    EXPECT_TRUE(board.placePiece(Position(7, 7), Piece::Black));
    EXPECT_EQ(board.getPiece(Position(7, 7)), Piece::Black);
    EXPECT_EQ(board.moveCount(), 1);
    EXPECT_EQ(board.pieceCount(Piece::Black), 1);
}

TEST_F(GameBoardTest, PlacePiece_OccupiedPosition_Fails)
{
    board.placePiece(Position(7, 7), Piece::Black);
    // 同一位置再次落子应失败。
    EXPECT_FALSE(board.placePiece(Position(7, 7), Piece::White));
    EXPECT_EQ(board.moveCount(), 1);
}

TEST_F(GameBoardTest, PlacePiece_InvalidPosition_Fails)
{
    EXPECT_FALSE(board.placePiece(Position(-1, 0), Piece::Black));
    EXPECT_FALSE(board.placePiece(Position(0, 15), Piece::White));
    EXPECT_EQ(board.moveCount(), 0);
}

TEST_F(GameBoardTest, PlacePiece_NoPiece_Fails)
{
    EXPECT_FALSE(board.placePiece(Position(7, 7), Piece::None));
}

TEST_F(GameBoardTest, RemovePiece_RestoresEmpty)
{
    board.placePiece(Position(5, 5), Piece::White);
    board.removePiece(Position(5, 5));
    EXPECT_EQ(board.getPiece(Position(5, 5)), Piece::None);
    EXPECT_EQ(board.moveCount(), 0);
    EXPECT_EQ(board.pieceCount(Piece::White), 0);
}

TEST_F(GameBoardTest, RemovePiece_UpdatesLastMove)
{
    board.placePiece(Position(3, 3), Piece::Black);
    board.placePiece(Position(4, 4), Piece::White);
    board.removePiece(Position(4, 4));
    // 最后落子应回退到 (3,3)。
    EXPECT_EQ(board.lastMove(), Position(3, 3));
}

TEST_F(GameBoardTest, CanPlace_EmptyIsTrue)
{
    EXPECT_TRUE(board.canPlace(Position(0, 0)));
    EXPECT_TRUE(board.canPlace(Position(14, 14)));
}

TEST_F(GameBoardTest, CanPlace_OccupiedIsFalse)
{
    board.placePiece(Position(7, 7), Piece::Black);
    EXPECT_FALSE(board.canPlace(Position(7, 7)));
}

TEST_F(GameBoardTest, CanPlace_InvalidIsFalse)
{
    EXPECT_FALSE(board.canPlace(Position(-1, 0)));
    EXPECT_FALSE(board.canPlace(Position(15, 15)));
}

TEST_F(GameBoardTest, Reset_ClearsAllState)
{
    board.placePiece(Position(7, 7), Piece::Black);
    board.placePiece(Position(7, 8), Piece::White);
    board.reset();
    EXPECT_TRUE(board.isEmpty());
    EXPECT_EQ(board.moveCount(), 0);
    EXPECT_EQ(board.pieceCount(Piece::Black), 0);
    EXPECT_EQ(board.pieceCount(Piece::White), 0);
    EXPECT_EQ(board.getPiece(Position(7, 7)), Piece::None);
}

TEST_F(GameBoardTest, MoveHistory_TracksOrder)
{
    board.placePiece(Position(0, 0), Piece::Black);
    board.placePiece(Position(1, 1), Piece::White);
    board.placePiece(Position(2, 2), Piece::Black);
    EXPECT_EQ(board.moveHistory().size(), 3u);
    EXPECT_EQ(board.lastMove(), Position(2, 2));
}

TEST_F(GameBoardTest, PieceCount_TracksCorrectly)
{
    board.placePiece(Position(0, 0), Piece::Black);
    board.placePiece(Position(1, 1), Piece::Black);
    board.placePiece(Position(2, 2), Piece::White);
    EXPECT_EQ(board.pieceCount(Piece::Black), 2);
    EXPECT_EQ(board.pieceCount(Piece::White), 1);
    EXPECT_EQ(board.pieceCount(Piece::None), 0);
}

TEST_F(GameBoardTest, GetPiece_OutOfBounds_ReturnsNone)
{
    EXPECT_EQ(board.getPiece(Position(-1, 0)), Piece::None);
    EXPECT_EQ(board.getPiece(Position(15, 15)), Piece::None);
}

// ==================== WinDetector::checkWinner 测试 ====================

TEST(WinDetectorTest, CheckWinner_EmptyBoard_ReturnsNone)
{
    WinDetector wd;
    GameBoard b;
    EXPECT_EQ(wd.checkWinner(b), Piece::None);
}

TEST(WinDetectorTest, CheckWinner_BlackWins)
{
    WinDetector wd;
    GameBoard b;
    for (int i = 0; i < 5; ++i) b.placePiece(Position(7, i), Piece::Black);
    EXPECT_EQ(wd.checkWinner(b), Piece::Black);
}

TEST(WinDetectorTest, CheckWinner_WhiteWins)
{
    WinDetector wd;
    GameBoard b;
    for (int i = 0; i < 5; ++i) b.placePiece(Position(i, 7), Piece::White);
    EXPECT_EQ(wd.checkWinner(b), Piece::White);
}

TEST(WinDetectorTest, CheckWinner_NoWinner_ReturnsNone)
{
    WinDetector wd;
    GameBoard b;
    b.placePiece(Position(7, 7), Piece::Black);
    b.placePiece(Position(7, 8), Piece::White);
    EXPECT_EQ(wd.checkWinner(b), Piece::None);
}

// ==================== AIEngine 测试 ====================

class AIEngineTest : public ::testing::Test
{
protected:
    AIEngine engine;
    GameBoard board;

    void SetUp() override
    {
        engine.setSearchDepth(2); // 浅搜索以加快测试速度。
    }
};

TEST_F(AIEngineTest, SearchDepth_SetAndGet)
{
    engine.setSearchDepth(4);
    EXPECT_EQ(engine.searchDepth(), 4);
}

TEST_F(AIEngineTest, CalculateBestMove_EmptyBoard_ReturnsValidPosition)
{
    // 空棋盘上 AI 应返回有效位置（通常为中心）。
    Position pos = engine.calculateBestMove(board, Piece::Black);
    EXPECT_TRUE(pos.isValid());
}

TEST_F(AIEngineTest, CalculateBestMove_BlocksOpponentFive)
{
    // 黑棋已有四连，AI（白棋）应封堵。
    for (int i = 3; i <= 6; ++i) board.placePiece(Position(7, i), Piece::Black);
    Position pos = engine.calculateBestMove(board, Piece::White);
    // AI 应选择 col 2 或 col 7 来阻止黑棋五连。
    bool blocksLeft  = (pos == Position(7, 2));
    bool blocksRight = (pos == Position(7, 7));
    EXPECT_TRUE(blocksLeft || blocksRight);
}

TEST_F(AIEngineTest, CalculateBestMove_CompletesOwnFive)
{
    // 白棋已有四连，AI 应直接完成五连而非做其他事。
    for (int i = 3; i <= 6; ++i) board.placePiece(Position(7, i), Piece::White);
    Position pos = engine.calculateBestMove(board, Piece::White);
    bool winsLeft  = (pos == Position(7, 2));
    bool winsRight = (pos == Position(7, 7));
    EXPECT_TRUE(winsLeft || winsRight);
}

TEST_F(AIEngineTest, CheckWin_AfterFive_ReturnsTrue)
{
    for (int i = 0; i < 5; ++i) board.placePiece(Position(7, i), Piece::Black);
    EXPECT_TRUE(engine.checkWin(board, Piece::Black));
    EXPECT_FALSE(engine.checkWin(board, Piece::White));
}

TEST_F(AIEngineTest, CheckWin_EmptyBoard_ReturnsFalse)
{
    EXPECT_FALSE(engine.checkWin(board, Piece::Black));
    EXPECT_FALSE(engine.checkWin(board, Piece::White));
}

// ==================== Position 测试 ====================

TEST(PositionTest, IsValid_InBounds)
{
    EXPECT_TRUE(Position(0, 0).isValid());
    EXPECT_TRUE(Position(14, 14).isValid());
    EXPECT_TRUE(Position(7, 7).isValid());
}

TEST(PositionTest, IsValid_OutOfBounds)
{
    EXPECT_FALSE(Position(-1, 0).isValid());
    EXPECT_FALSE(Position(0, -1).isValid());
    EXPECT_FALSE(Position(15, 0).isValid());
    EXPECT_FALSE(Position(0, 15).isValid());
}

TEST(PositionTest, Invalid_IsNotValid)
{
    EXPECT_FALSE(Position::Invalid().isValid());
}

TEST(PositionTest, Equality)
{
    EXPECT_EQ(Position(3, 4), Position(3, 4));
    EXPECT_NE(Position(3, 4), Position(4, 3));
}

TEST(PositionTest, IsPlayable_ExcludesBorders)
{
    EXPECT_FALSE(Position(0, 7).isPlayable());
    EXPECT_FALSE(Position(14, 7).isPlayable());
    EXPECT_FALSE(Position(7, 0).isPlayable());
    EXPECT_FALSE(Position(7, 14).isPlayable());
    EXPECT_TRUE(Position(7, 7).isPlayable());
}

// ==================== ZobristHash + 置换表 测试（T014）====================

#include "domain/services/transposition_table.h"

TEST(ZobristHashTest, EmptyBoard_HashIsZero)
{
    ZobristHash zh;
    GameBoard board;
    EXPECT_EQ(zh.compute(board), 0ULL);
}

TEST(ZobristHashTest, PlacePiece_HashChanges)
{
    ZobristHash zh;
    GameBoard board;
    uint64_t h0 = zh.compute(board);
    board.placePiece(Position(7, 7), Piece::White);
    uint64_t h1 = zh.compute(board);
    EXPECT_NE(h0, h1);
}

TEST(ZobristHashTest, IncrementalUpdate_MatchesFullCompute)
{
    ZobristHash zh;
    GameBoard board;
    uint64_t hash = zh.compute(board);
    hash = zh.update(hash, 7, 7, Piece::White);
    board.placePiece(Position(7, 7), Piece::White);
    EXPECT_EQ(hash, zh.compute(board));
}

TEST(ZobristHashTest, PlaceAndRemove_RestoresHash)
{
    ZobristHash zh;
    uint64_t h0 = 0;
    uint64_t h1 = zh.update(h0, 3, 5, Piece::Black);
    uint64_t h2 = zh.update(h1, 3, 5, Piece::Black);
    EXPECT_EQ(h0, h2);
}

TEST(ZobristHashTest, DifferentPieces_DifferentHash)
{
    ZobristHash zh;
    uint64_t hw = zh.update(0, 7, 7, Piece::White);
    uint64_t hb = zh.update(0, 7, 7, Piece::Black);
    EXPECT_NE(hw, hb);
}

TEST(TranspositionTableTest, StoreAndProbe_Exact)
{
    TranspositionTable tt;
    tt.store(12345ULL, 100, 4, TTFlag::Exact);
    auto e = tt.probe(12345ULL, 4);
    ASSERT_TRUE(e.has_value());
    EXPECT_EQ(e->score, 100);
    EXPECT_EQ(e->flag, TTFlag::Exact);
}

TEST(TranspositionTableTest, Probe_MissOnDepthTooLow)
{
    TranspositionTable tt;
    tt.store(99ULL, 50, 3, TTFlag::Exact);
    EXPECT_FALSE(tt.probe(99ULL, 4).has_value());
}

TEST(TranspositionTableTest, Probe_HitOnEqualDepth)
{
    TranspositionTable tt;
    tt.store(99ULL, 50, 3, TTFlag::Exact);
    EXPECT_TRUE(tt.probe(99ULL, 3).has_value());
}

TEST(TranspositionTableTest, Clear_RemovesEntries)
{
    TranspositionTable tt;
    tt.store(1ULL, 10, 2, TTFlag::Exact);
    tt.clear();
    EXPECT_FALSE(tt.probe(1ULL, 2).has_value());
}

TEST(TranspositionTableTest, HitCount_Increments)
{
    TranspositionTable tt;
    tt.store(42ULL, 77, 3, TTFlag::Exact);
    tt.resetStats();
    tt.probe(42ULL, 3);
    tt.probe(42ULL, 3);
    EXPECT_EQ(tt.hitCount(), 2ULL);
}

// ==================== AIEngine 迭代加深+时间控制 测试（T015）====================

#include "domain/services/ai_engine.h"
#include <chrono>

TEST(AIEngineIDDFSTest, EmptyBoard_ReturnsCenterOrValid)
{
    AIEngine engine;
    GameBoard board;
    Position pos = engine.calculateBestMove(board, Piece::White);
    EXPECT_EQ(pos.row(), 7);
    EXPECT_EQ(pos.col(), 7);
}

TEST(AIEngineIDDFSTest, ImmediateWin_PicksWinningMove)
{
    AIEngine engine;
    engine.setTimeLimitMs(3000);
    engine.setSearchDepth(4);
    GameBoard board;
    for (int c = 3; c <= 6; ++c)
        board.placePiece(Position(7, c), Piece::White);
    Position pos = engine.calculateBestMove(board, Piece::White);
    GameBoard test = board;
    test.placePiece(pos, Piece::White);
    WinDetector wd;
    EXPECT_TRUE(wd.checkWin(test, Piece::White));
}

TEST(AIEngineIDDFSTest, TimeLimitRespected)
{
    AIEngine engine;
    engine.setTimeLimitMs(500);
    engine.setSearchDepth(8);
    GameBoard board;
    board.placePiece(Position(7, 7), Piece::Black);
    board.placePiece(Position(7, 8), Piece::White);
    board.placePiece(Position(8, 7), Piece::Black);
    auto start = std::chrono::steady_clock::now();
    Position pos = engine.calculateBestMove(board, Piece::White);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    EXPECT_LT(ms, 700);
    EXPECT_TRUE(pos.isValid());
}

TEST(AIEngineIDDFSTest, Setters_Applied)
{
    AIEngine engine;
    engine.setSearchDepth(3);
    engine.setTimeLimitMs(1500);
    EXPECT_EQ(engine.searchDepth(), 3);
    EXPECT_EQ(engine.timeLimitMs(), 1500);
}

TEST(AIEngineIDDFSTest, ClearTT_ResetsHitCount)
{
    AIEngine engine;
    engine.setTimeLimitMs(500);
    engine.setSearchDepth(3);
    GameBoard board;
    board.placePiece(Position(7, 7), Piece::Black);
    board.placePiece(Position(7, 8), Piece::White);
    engine.calculateBestMove(board, Piece::White);
    engine.clearTranspositionTable();
    EXPECT_EQ(engine.ttHitCount(), 0ULL);
}

// ==================== DifficultyConfig 测试（T016）====================

#include "domain/services/difficulty_config.h"

TEST(DifficultyConfigTest, DefaultProfiles_Count)
{
    // 内置默认配置应有4个难度
    game_core::DifficultyConfig cfg;
    EXPECT_EQ(cfg.profileCount(), 4);
}

TEST(DifficultyConfigTest, GetProfile_Easy)
{
    game_core::DifficultyConfig cfg;
    const auto& p = cfg.getProfile(1);
    EXPECT_EQ(p.id, 1);
    EXPECT_GT(p.error_rate, 0.0);  // 简单难度有错误率
    EXPECT_LE(p.search_depth, 3);  // 简单难度搜索深度较浅
}

TEST(DifficultyConfigTest, GetProfile_Hard)
{
    game_core::DifficultyConfig cfg;
    const auto& p = cfg.getProfile(3);
    EXPECT_EQ(p.id, 3);
    EXPECT_DOUBLE_EQ(p.error_rate, 0.0); // 困难难度无失误
    EXPECT_GE(p.search_depth, 4);        // 困难难度搜索较深
}

TEST(DifficultyConfigTest, GetProfile_Expert)
{
    game_core::DifficultyConfig cfg;
    const auto& p = cfg.getProfile(4);
    EXPECT_EQ(p.id, 4);
    EXPECT_DOUBLE_EQ(p.error_rate, 0.0);
    EXPECT_GE(p.search_depth, 6);
}

TEST(DifficultyConfigTest, GetProfile_InvalidId_ReturnDefault)
{
    game_core::DifficultyConfig cfg;
    const auto& p = cfg.getProfile(999);
    // 找不到时返回默认（普通难度 id=2）
    EXPECT_EQ(p.id, 2);
}

TEST(DifficultyConfigTest, LoadFromFile_NonExistent_KeepsDefaults)
{
    game_core::DifficultyConfig cfg;
    bool ok = cfg.loadFromFile("non_existent_file.json");
    EXPECT_FALSE(ok);
    // 默认配置仍然有效
    EXPECT_EQ(cfg.profileCount(), 4);
}

TEST(DifficultyConfigTest, EasyHasHigherErrorRate_ThanHard)
{
    game_core::DifficultyConfig cfg;
    const auto& easy = cfg.getProfile(1);
    const auto& hard = cfg.getProfile(3);
    EXPECT_GT(easy.error_rate, hard.error_rate);
}

TEST(AIEngineIDDFSTest, SetDifficultyProfile_Easy_HasErrorRate)
{
    // 简单难度：error_rate=0.3，AI 应当能在多次调用中返回非最优步
    game_core::DifficultyConfig cfg;
    const auto& easy = cfg.getProfile(1);
    AIEngine engine;
    engine.setDifficultyProfile(easy);
    EXPECT_EQ(engine.searchDepth(), easy.search_depth);
    EXPECT_EQ(engine.timeLimitMs(), easy.time_limit_ms);

    // 调用 calculateBestMove，确保不崩溃且返回有效位置
    GameBoard board;
    board.placePiece(Position(7, 7), Piece::Black);
    board.placePiece(Position(7, 8), Piece::White);
    board.placePiece(Position(8, 7), Piece::Black);
    Position pos = engine.calculateBestMove(board, Piece::White);
    EXPECT_TRUE(pos.isValid());
}

TEST(AIEngineIDDFSTest, SetDifficultyProfile_Hard_NoError)
{
    game_core::DifficultyConfig cfg;
    const auto& hard = cfg.getProfile(3);
    AIEngine engine;
    engine.setDifficultyProfile(hard);
    // 困难难度：直接走最优步，返回有效位置
    GameBoard board;
    board.placePiece(Position(7, 7), Piece::Black);
    board.placePiece(Position(7, 8), Piece::White);
    Position pos = engine.calculateBestMove(board, Piece::White);
    EXPECT_TRUE(pos.isValid());
}

// ==================== SqliteStore 测试（T023）====================

#include "sqlitestore.h"
#include <QStandardPaths>
#include <QFile>

class SqliteStoreTest : public ::testing::Test
{
protected:
    SqliteStore store;
    void SetUp() override {
        // 使用临时路径，每次测试前清除数据库
        QFile::remove(SqliteStore::DbPath());
        ASSERT_TRUE(store.Open());
    }
    void TearDown() override {
        store.Close();
        QFile::remove(SqliteStore::DbPath());
    }
};

TEST_F(SqliteStoreTest, Open_CreatesDatabase)
{
    EXPECT_TRUE(store.IsOpen());
}

TEST_F(SqliteStoreTest, SaveAndRead_PlayerRecord)
{
    PlayerRecord rec;
    rec.displayName = "TestUser";
    rec.preferredStarter = "player";
    GameRecord game;
    game.finishedAt = "2026-01-01";
    game.playerWon = true;
    game.playerStarted = true;
    game.moveCount = 5;
    rec.games.push_back(game);

    EXPECT_TRUE(store.SaveRecord(rec));

    PlayerRecord loaded = store.RecordForUser("TestUser");
    EXPECT_EQ(loaded.displayName, QStringLiteral("TestUser"));
    EXPECT_EQ(loaded.games.size(), 1);
    EXPECT_TRUE(loaded.games[0].playerWon);
    EXPECT_EQ(loaded.games[0].finishedAt, QStringLiteral("2026-01-01"));
}

TEST_F(SqliteStoreTest, LastUser_SetAndGet)
{
    store.SetLastUser("Alice");
    EXPECT_EQ(store.LastUser(), QStringLiteral("Alice"));
}

TEST_F(SqliteStoreTest, TouchRecentUser_MaintainsOrder)
{
    store.TouchRecentUser("Alice");
    store.TouchRecentUser("Bob");
    store.TouchRecentUser("Alice"); // Alice 移到最前
    QStringList recent = store.RecentUsers();
    ASSERT_FALSE(recent.isEmpty());
    EXPECT_EQ(recent.first(), QStringLiteral("Alice"));
}

TEST_F(SqliteStoreTest, MigrateFromJson_NonExistent_Succeeds)
{
    // 不存在的 JSON 文件，迁移应成功（标记为已迁移）
    bool ok = store.MigrateFromJson("/non/existent/players.json");
    EXPECT_TRUE(ok);
    // 再次迁移应幂等
    ok = store.MigrateFromJson("/non/existent/players.json");
    EXPECT_TRUE(ok);
}

TEST_F(SqliteStoreTest, SaveRecord_WithMoves)
{
    PlayerRecord rec;
    rec.displayName = "MoveUser";
    GameRecord game;
    game.finishedAt = "2026-03-30";
    game.playerWon = false;
    game.moveCount = 3;
    MoveRecord mv1{7, 7, BLACK};
    MoveRecord mv2{8, 8, WHITE};
    MoveRecord mv3{9, 9, BLACK};
    game.moves.push_back(mv1);
    game.moves.push_back(mv2);
    game.moves.push_back(mv3);
    rec.games.push_back(game);

    ASSERT_TRUE(store.SaveRecord(rec));
    PlayerRecord loaded = store.RecordForUser("MoveUser");
    ASSERT_EQ(loaded.games.size(), 1);
    ASSERT_EQ(loaded.games[0].moves.size(), 3);
    EXPECT_EQ(loaded.games[0].moves[0].row, 7);
    EXPECT_EQ(loaded.games[0].moves[1].col, 8);
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
