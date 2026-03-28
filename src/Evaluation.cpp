#include "Evaluation.h"

Evaluation::Evaluation(void)
{
}


Evaluation::~Evaluation(void)
{
}

int Evaluation::EvaluateBoard(ePiece (&arrBoard)[15][15])
{
	int nScore = 0;
	QVector<QVector<ePiece>> vecSum = CutBoard(arrBoard);

	nScore = Score(arrBoard, WHITE, vecSum) - Score(arrBoard, BLACK, vecSum);

	return nScore;
}

QVector<QVector<ePiece>> Evaluation::CutBoard(ePiece (&arrBoard)[15][15])
{
	QVector<QVector<ePiece>> vecSum;

	//横
	for(int j = 0; j < 15; j++)
	{
		QVector<ePiece> vec;
		for(int i = 0; i< 15; i++)
		{
			vec.push_back(arrBoard[i][j]);
		}
		vecSum.push_back(vec);
	}

	//竖
	for(int j = 0; j < 15; j++)
	{
		QVector<ePiece> vec;
		for(int i = 0; i< 15; i++)
		{
			vec.push_back(arrBoard[j][i]);
		}
		vecSum.push_back(vec);
	}

	//左上到右下
	for(int j = 14; j>=0; --j)
	{
		QVector<ePiece> vec;
		int m = 0; 
		int n = j; 
		while(n<15)
		{
			vec.push_back(arrBoard[m][n]);
			++n;
			++m;
		}
		vecSum.push_back(vec);
	}
	for(int i = 1; i< 15; ++i)
	{
		QVector<ePiece> vec;
		int m = i; 
		int n = 0; 
		while(m<15)
		{
			vec.push_back(arrBoard[m][n]);
			++n;
			++m;
		}
		vecSum.push_back(vec);
	}

	//右上到左下
	for(int j = 14; j>=0; --j)
	{
		QVector<ePiece> vec;
		int m = 14; 
		int n = j; 
		while(n<15)
		{
			vec.push_back(arrBoard[m][n]);
			++n;
			--m;
		}
		vecSum.push_back(vec);
	}
	for(int i = 13; i >= 0; --i)
	{
		QVector<ePiece> vec;
		int m = i; 
		int n = 0; 
		while(m>=0)
		{
			vec.push_back(arrBoard[m][n]);
			++n;
			--m;
		}
		vecSum.push_back(vec);
	}

	return vecSum;
}

int Evaluation::Score(ePiece (&arrBoard)[15][15], ePiece piece, QVector<QVector<ePiece>> &vecSum)
{
	int nSocre = 0;
	for(int i = 0; i< vecSum.size(); ++i)
	{
		for(int j = 0; j< vecSum[i].size(); ++j)
		{
			if(vecSum[i][j] == piece)
			{
				if(IsOpenOne(vecSum[i], j, piece))
				{
					nSocre += OPEN_ONE;
				}
				else if(IsOpenTwo(vecSum[i], j, piece))
				{
					nSocre += OPEN_TWO;
				}
				else if(IsOpenThree(vecSum[i], j, piece))
				{
					nSocre += OPEN_THREE;
				}
				else if(IsOpenFour(vecSum[i], j, piece))
				{
					nSocre += OPEN_FOUR;
				}
				else if(IsFive(vecSum[i], j, piece))
				{
					nSocre += FIVE;
				}
				else if(IsCloseOne(vecSum[i], j, piece))
				{
					nSocre += CLOSE_ONE;
				}
				else if(IsCloseTwO(vecSum[i], j, piece))
				{
					nSocre += CLOSE_TWO;
				}
				else if(IsCloseThree(vecSum[i], j, piece))
				{
					nSocre += CLOSE_THREE;
				}
				else if(IsCloseFour(vecSum[i], j, piece))
				{
					nSocre += CLOSE_FOUR;
				}
				else
					nSocre += 0;
			}

		}
	}
	return nSocre;
}

bool Evaluation::IsOpenOne(QVector<ePiece> vec, int index, ePiece piece)
{
	if(index-1>= 0 && index+1 <= vec.size()-1)
	{
		if(vec[index-1] == NONE && vec[index+1] == NONE && vec[index] == piece)
		{
			return true;
		}
	}
	return false;
}

bool Evaluation::IsOpenTwo(QVector<ePiece> vec, int index, ePiece piece)
{
	if(index-1>= 0 && index+2 <= vec.size()-1)
	{
		if(vec[index-1] == NONE &&
			vec[index+2] == NONE &&
			vec[index] == piece &&
			vec[index+1] == piece)
		{
			return true;
		}
	}
	return false;
}

bool Evaluation::IsOpenThree(QVector<ePiece> vec, int index, ePiece piece)
{
	if(index-1>= 0 && index+3 <= vec.size()-1)
	{
		if(vec[index-1] == NONE &&
			vec[index+3] == NONE &&
			vec[index] == piece &&
			vec[index+1] == piece &&
			vec[index+2] == piece)
		{
			return true;
		}
	}
	return false;
}

bool Evaluation::IsOpenFour(QVector<ePiece> vec, int index, ePiece piece)
{
	if(index-1>= 0 && index+4 <= vec.size()-1)
	{
		if(vec[index-1] == NONE &&
			vec[index+4] == NONE &&
			vec[index] == piece &&
			vec[index+1] == piece &&
			vec[index+2] == piece &&
			vec[index+3] == piece)
		{
			return true;
		}
	}
	return false;
}

bool Evaluation::IsFive(QVector<ePiece> vec, int index, ePiece piece)
{
	if(index+4 <= vec.size()-1)
	{
		if(	vec[index+4] == piece &&
			vec[index] == piece &&
			vec[index+1] == piece &&
			vec[index+2] == piece &&
			vec[index+3] == piece)
		{
			return true;
		}
	}
	return false;
}

bool Evaluation::IsCloseOne(QVector<ePiece> vec, int index, ePiece piece)
{
	if(vec.size()>1 && index == 0 && vec[index] == piece && vec[index+1] == NONE )  //在最左边
	{
		return true;
	}
	else if(vec.size()>1 && index == vec.size()-1 && vec[index] == piece && vec[index-1] == NONE)  //在最右边
	{
		return true;
	}
	else if(vec.size()>2 && index-1>=0 && index+1<=vec.size()-1 && vec[index] == piece)  //在中间
	{
		if(vec[index-1] != piece && vec[index-1] != NONE && vec[index+1] == NONE)
		{
			return true;
		}
		else if(vec[index+1] != piece && vec[index+1] != NONE && vec[index-1] == NONE)
		{
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool Evaluation::IsCloseTwO(QVector<ePiece> vec, int index, ePiece piece)
{
	if(vec.size()>2 && index == 0 && vec[index] == piece && vec[index+1] == piece && vec[index+2] == NONE )  //在最左边
	{
		return true;
	}
	else if(vec.size()>2 && index == vec.size()-1 && vec[index] == piece && vec[index-1] == piece && vec[index-2] == NONE)  //在最右边
	{
		return true;
	}
	else if(vec.size()>3 && index-1>=0 && index+2<=vec.size()-1 && vec[index] == piece&& vec[index+1] == piece)  //在中间
	{
		if(vec[index-1] != piece && vec[index-1] != NONE && vec[index+2] == NONE)
		{
			return true;
		}
		else if(vec[index+2] != piece && vec[index+2] != NONE && vec[index-1] == NONE)
		{
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool Evaluation::IsCloseThree(QVector<ePiece> vec, int index, ePiece piece)
{
	if(vec.size()>3 && index == 0 && vec[index] == piece && vec[index+1] == piece && vec[index+2] == piece &&vec[index+3] == NONE)  //在最左边
	{
		return true;
	}
	else if(vec.size()>3 && index == vec.size()-1 && vec[index] == piece && vec[index-1] == piece && vec[index-2] == piece && vec[index-3] == NONE)  //在最右边
	{
		return true;
	}
	else if(vec.size()>4 && index-1>=0 && index+3<=vec.size()-1 && vec[index] == piece&& vec[index+1] == piece&& vec[index+2] == piece)  //在中间
	{
		if(vec[index-1] != piece && vec[index-1] != NONE && vec[index+3] == NONE)
		{
			return true;
		}
		else if(vec[index+3] != piece && vec[index+3] != NONE && vec[index-1] == NONE)
		{
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool Evaluation::IsCloseFour(QVector<ePiece> vec, int index, ePiece piece)
{
	if(vec.size()>4 && index == 0 && vec[index] == piece && vec[index+1] == piece && vec[index+2] == piece&& vec[index+3] == piece &&vec[index+4] == NONE)  //在最左边
	{
		return true;
	}
	else if(vec.size()>4 && index == vec.size()-1 && vec[index] == piece && vec[index-1] == piece && vec[index-2] == piece&& vec[index-3] == piece && vec[index-4] == NONE)  //在最右边
	{
		return true;
	}
	else if(vec.size()>5 && index-1>=0 && index+4<=vec.size()-1 && vec[index] == piece&& vec[index+1] == piece&& vec[index+2] == piece&& vec[index+3] == piece)  //在中间
	{
		if(vec[index-1] != piece && vec[index-1] != NONE && vec[index+4] == NONE)
		{
			return true;
		}
		else if(vec[index+4] != piece && vec[index+4] != NONE && vec[index-1] == NONE)
		{
			return true;
		}
		else
			return false;
	}
	else
		return false;
}
