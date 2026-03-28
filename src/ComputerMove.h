#pragma once
#include"types.h"

#include<QVector>
class judgeWinner;
class Evaluation;

class ComputerMove
{
public:
	ComputerMove();
	~ComputerMove(void);

	int X(){return x;}
	int Y(){return y;}

	void Computer_1(ePiece (&arrBoard)[15][15]); //最初级，随意落子

	void MaxMinSearch(ePiece (&arrBoard)[15][15], int deep);  //极大极小值搜索

private:

	QVector<QVector<int>> GenCandidator(ePiece (&arrBoard)[15][15], ePiece piece);   //可以落子的位置

	int MinSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta);
	int MaxSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta);

	int x;
	int y;

	judgeWinner * m_pJudgeWinner;
	Evaluation *m_pEvalution;
};

