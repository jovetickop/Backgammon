#include "judgeWinner.h"


judgeWinner::judgeWinner(void)
{

}


judgeWinner::~judgeWinner(void)
{
}

bool judgeWinner::IsWon(ePiece piece, ePiece (&array)[15][15])
{

	m_piece = piece;
	for(int i = 0; i< 15; ++i)
	{
		for(int j = 0; j< 15; ++j)
		{
			m_arrBoard[i][j] = array[i][j];
		}
	}

	if(SituationHor() || SituationLef() || SituationRig() || SituationVer())
		return true;
	else
		return false;
}

bool judgeWinner::SituationHor()
{
	for(int i = 0; i<11; ++i)
	{
		for(int j = 0; j< 15; ++j)
		{
			if(m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i+1][j] == m_piece &&
				m_arrBoard[i+2][j] == m_piece &&
				m_arrBoard[i+3][j] == m_piece &&
				m_arrBoard[i+4][j] == m_piece )
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationVer()
{
	for(int i = 0; i<15; ++i)
	{
		for(int j = 0; j< 11; ++j)
		{
			if(m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i][j+1] == m_piece &&
				m_arrBoard[i][j+2] == m_piece &&
				m_arrBoard[i][j+3] == m_piece &&
				m_arrBoard[i][j+4] == m_piece )
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationLef()
{
	for(int i = 4; i<15; ++i)
	{
		for(int j = 0; j< 11; ++j)
		{
			if(m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i-1][j+1] == m_piece &&
				m_arrBoard[i-2][j+2] == m_piece &&
				m_arrBoard[i-3][j+3] == m_piece &&
				m_arrBoard[i-4][j+4] == m_piece )
			{
				return true;
			}
		}
	}
	return false;
}

bool judgeWinner::SituationRig()
{
	for(int i = 0; i<11; ++i)
	{
		for(int j = 0; j< 11; ++j)
		{
			if(m_arrBoard[i][j] == m_piece &&
				m_arrBoard[i+1][j+1] == m_piece &&
				m_arrBoard[i+2][j+2] == m_piece &&
				m_arrBoard[i+3][j+3] == m_piece &&
				m_arrBoard[i+4][j+4] == m_piece )
			{
				return true;
			}
		}
	}
	return false;
}
