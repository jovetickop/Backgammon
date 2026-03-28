#pragma once
#include "types.h"

// 胜负判断器：在 15x15 棋盘上检测是否形成五连。
class judgeWinner
{
public:
	judgeWinner(void);
	~judgeWinner(void);

	// 对外入口：判断指定棋子是否已经获胜。
	bool IsWon(ePiece piece, ePiece (&array)[15][15]);

	// 四个方向的五连检测。
	bool SituationHor();
	bool SituationVer();
	bool SituationLef();
	bool SituationRig();

private:
	ePiece m_piece;
	ePiece m_arrBoard[15][15];
};
