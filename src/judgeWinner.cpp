#include "judgeWinner.h"

judgeWinner::judgeWinner(void)
{
}

judgeWinner::~judgeWinner(void)
{
}

bool judgeWinner::IsWon(ePiece piece, ePiece (&array)[15][15])
{
	// 先缓存当前要判断的棋子类型与棋盘快照。
	m_piece = piece;
	for (int i = 0; i < 15; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			m_arrBoard[i][j] = array[i][j];
		}
	}

	// 任一方向出现五连即判胜。
	return SituationHor() || SituationLef() || SituationRig() || SituationVer();
}

bool judgeWinner::SituationHor()
{
	// 横向检测。
	for (int i = 0; i < 11; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			if (m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i + 1][j] == m_piece &&
				m_arrBoard[i + 2][j] == m_piece &&
				m_arrBoard[i + 3][j] == m_piece &&
				m_arrBoard[i + 4][j] == m_piece)
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationVer()
{
	// 纵向检测。
	for (int i = 0; i < 15; ++i)
	{
		for (int j = 0; j < 11; ++j)
		{
			if (m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i][j + 1] == m_piece &&
				m_arrBoard[i][j + 2] == m_piece &&
				m_arrBoard[i][j + 3] == m_piece &&
				m_arrBoard[i][j + 4] == m_piece)
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationLef()
{
	// 左下方向斜线检测。
	for (int i = 4; i < 15; ++i)
	{
		for (int j = 0; j < 11; ++j)
		{
			if (m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i - 1][j + 1] == m_piece &&
				m_arrBoard[i - 2][j + 2] == m_piece &&
				m_arrBoard[i - 3][j + 3] == m_piece &&
				m_arrBoard[i - 4][j + 4] == m_piece)
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationRig()
{
	// 右下方向斜线检测。
	for (int i = 0; i < 11; ++i)
	{
		for (int j = 0; j < 11; ++j)
		{
			if (m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i + 1][j + 1] == m_piece &&
				m_arrBoard[i + 2][j + 2] == m_piece &&
				m_arrBoard[i + 3][j + 3] == m_piece &&
				m_arrBoard[i + 4][j + 4] == m_piece)
			{
				return true;
			}
		}
	}
	return false;
}
