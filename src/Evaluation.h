#pragma once

#include"types.h"
#include<QVector>
/************************************************************************/
/* 
将棋盘分成横竖斜四种，每一行看成一个vector,再对每一行评分
*/
/************************************************************************/

class Evaluation
{
public:
	Evaluation(void);
	~Evaluation(void);
	int EvaluateBoard(ePiece (&arrBoard)[15][15]);

private:

	//将棋盘按照横竖斜分成四种情况，将每一行都变成一个集合
	QVector<QVector<ePiece>> CutBoard(ePiece (&arrBoard)[15][15]);  //分割棋盘成多个一维数组
	
	//对当前某种棋子的局面评分
	int Score(ePiece (&arrBoard)[15][15], ePiece piece, QVector<QVector<ePiece>> &vecSum);  //对每种棋子评分
	
	//判断当前行的棋子排列情况
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

