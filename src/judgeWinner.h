#pragma once
#include "types.h"
class judgeWinner
{
public:
	judgeWinner(void);
	~judgeWinner(void);
	bool IsWon(ePiece piece, ePiece (&array)[15][15]);

	bool SituationHor();  //横着
	bool SituationVer();  //竖着
	bool SituationLef();  //向左下斜着
	bool SituationRig();  //向右下斜着
private:
	ePiece m_piece;
	ePiece m_arrBoard[15][15];
};

