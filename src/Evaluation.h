#pragma once

#include "types.h"
#include <QVector>

// 局面评估器：把棋盘拆成多个一维线段并统计棋型分值。
class Evaluation
{
public:
	Evaluation(void);
	~Evaluation(void);

	// 返回白方分 - 黑方分，分值越大越有利于白方（电脑）。
	int EvaluateBoard(ePiece (&arrBoard)[15][15]);

	// 检测指定棋子是否有"一步即可五连"的棋型（活四或冲四）。
	// 用于判断轮次方是否已经拥有必胜局面。
	bool HasWinningMove(ePiece (&arrBoard)[15][15], ePiece piece);

private:
	// 将棋盘按横、竖、两条斜线方向切分为一组一维线段。
	QVector<QVector<ePiece>> CutBoard(ePiece (&arrBoard)[15][15]);

	// 统计指定棋子的总评分。
	int Score(ePiece (&arrBoard)[15][15], ePiece piece, QVector<QVector<ePiece>> &vecSum);

	// 各类棋型检测。
	bool IsOpenOne(QVector<ePiece> vec, int index, ePiece piece);
	bool IsOpenTwo(QVector<ePiece> vec, int index, ePiece piece);
	bool IsOpenThree(QVector<ePiece> vec, int index, ePiece piece);
	bool IsOpenFour(QVector<ePiece> vec, int index, ePiece piece);
	bool IsFive(QVector<ePiece> vec, int index, ePiece piece);
	bool IsCloseOne(QVector<ePiece> vec, int index, ePiece piece);
	bool IsCloseTwO(QVector<ePiece> vec, int index, ePiece piece);
	bool IsCloseThree(QVector<ePiece> vec, int index, ePiece piece);
	bool IsCloseFour(QVector<ePiece> vec, int index, ePiece piece);
};
