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

	// 检测指定棋子是否有活三（含跳活三：三子中有间隔但填充后可四连）。
	// 返回活三的"关键位置"数量（端点+间隔位置），数量 >=2 则对方无法同时封堵，属于必杀。
	// 返回 -1 表示无活三。
	int CountOpenThrees(ePiece (&arrBoard)[15][15], ePiece piece);

	// 快速评估单步落子后对指定棋子的增益分值（用于候选点排序）。
	// 返回正值表示有利于 piece，负值表示不利于 piece。
	int EvaluateMove(ePiece (&arrBoard)[15][15], int row, int col, ePiece piece);

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
