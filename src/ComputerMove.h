#pragma once
#include "types.h"
#include <QVector>

class judgeWinner;
class Evaluation;

// 电脑落子决策：候选点生成 + 极大极小搜索 + alpha-beta 剪枝。
class ComputerMove
{
public:
	ComputerMove();
	~ComputerMove(void);

	int X() { return x; }
	int Y() { return y; }

	// 随机落子（保留用于调试/对照）。
	void Computer_1(ePiece (&arrBoard)[15][15]);

	// 主决策入口。
	void MaxMinSearch(ePiece (&arrBoard)[15][15], int deep);

private:
	// 生成候选落子点（靠近已有棋子的位置）。
	QVector<QVector<int>> GenCandidator(ePiece (&arrBoard)[15][15], ePiece piece);

	// 搜索树最小层（对手回合）。
	int MinSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta);
	// 搜索树最大层（电脑回合）。
	int MaxSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta);

	int x;
	int y;

	judgeWinner *m_pJudgeWinner;
	Evaluation *m_pEvalution;
};
