#include "ComputerMove.h"
#include<QTime>
#include "judgeWinner.h"
#include "Evaluation.h"

ComputerMove::ComputerMove()
{
	x = 0;
	y = 0;

	m_pJudgeWinner = new judgeWinner;
	m_pEvalution = new Evaluation;
}


ComputerMove::~ComputerMove(void)
{
	delete m_pJudgeWinner;
	delete m_pEvalution;
}

void ComputerMove::Computer_1(ePiece (&arrBoard)[15][15])
{
	qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
	bool bOk = false;
	
	while(!bOk)
	{
		int randX = qrand()%15;
		int randY = qrand()%15;
		//if(m_arrBoard[randX][randY] == NONE && randX != 0 && randY != 0)
		if(arrBoard[randX][randY] == NONE)
		{
			x = randX;
			y = randY;
			bOk = true;
			return;
		}
	}

}


QVector<QVector<int>> ComputerMove::GenCandidator(ePiece (&arrBoard)[15][15], ePiece piece)
{
	QVector<QVector<int>> vecSum;
	for(int i = 0; i< 15; ++i)
	{
		for(int j = 0; j< 15; ++j)
		{
			//把空白的点当作候选
			//if(arrBoard[i][j] == NONE)
			//{
			//	QVector<int> vec;
			//	vec.push_back(i);
			//	vec.push_back(j);
			//	vecSum.push_back(vec);
			//}
			
			if(arrBoard[i][j] == NONE)
			{
				//某个点的周围八个点是否有棋子
				if((i-1>=0 && j-1>=0 && arrBoard[i-1][j-1] != NONE)||
					(i-1 >=0 &&arrBoard[i-1][j] != NONE) ||
					(i-1 >=0 && j+1<=14 && arrBoard[i-1][j+1] != NONE) ||
					(j-1>=0 && arrBoard[i][j-1] != NONE) ||
					(j+1<=14 && arrBoard[i][j+1] != NONE) ||
					(i+1<=14 && j-1>=0 && arrBoard[i+1][j-1] != NONE) ||
					(i+1<=14 && arrBoard[i+1][j] != NONE) ||
					(i+1<=14 && j+1 <=14 && arrBoard[i+1][j+1] != NONE))
				{
					QVector<int> vec;
					vec.push_back(i);
					vec.push_back(j);
					vecSum.push_back(vec);
				}
			}

			//if(arrBoard[i][j] == NONE)
			//{
			//	//三步之内有没有点
			//	bool bHasPoint = false;
			//	for(int m = 0; m<=3; ++m)
			//	{
			//		for(int n = 0; n<=3; ++n)
			//		{
			//			if(m+n<=3 && arrBoard[i+m][j+n] != NONE)
			//			{
			//				bHasPoint = true;
			//				break;
			//			}
			//			
			//		}
			//		if(bHasPoint)
			//			break;

			//	}
			//	if(bHasPoint)
			//	{
			//		QVector<int> vec;
			//		vec.push_back(i);
			//		vec.push_back(j);
			//		vecSum.push_back(vec);
			//	}
			//	
			//}

		}
	}
	return vecSum;
}



void ComputerMove::MaxMinSearch(ePiece (&arrBoard)[15][15], int deep)
{
	QVector<QVector<int>> vecCandidator =  GenCandidator(arrBoard, WHITE);

	QVector<QVector<int>> vecBestPoints;
	int nBestScore = 0x8fffffff;  //初值是最小的负数
	for(int i = 0; i< vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];        //待选的点
		arrBoard[vecPoint[0]][vecPoint[1]] = WHITE;     //机器下一个子
		//int nScore = MinSearch(arrBoard, deep-1, nBestScore>0x8fffffff ? nBestScore:0x8fffffff, 0x7fffffff);     //预估人类下子的分数
		int nScore = MinSearch(arrBoard, deep-1, 0x8fffffff, 0x7fffffff);     //预估人类下子的分数

		if(nScore == nBestScore)
		{
			vecBestPoints.push_back(vecPoint);
		}
		if(nScore >nBestScore)
		{
			nBestScore = nScore;
			vecBestPoints.clear();
			vecBestPoints.push_back(vecPoint);
		}
		arrBoard[vecPoint[0]][vecPoint[1]] = NONE;     //清除下的子
	}
	
	//在分数最高的几个点中随机选一个
	qsrand(QTime(0,0,0).msecsTo(QTime::currentTime()));
	int index = qrand()%vecBestPoints.size();
	x = vecBestPoints[index][0];
	y = vecBestPoints[index][1];
}

int ComputerMove::MinSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta)
{
	int nScore = m_pEvalution->EvaluateBoard(arrBoard);
	if(deep <=0 || m_pJudgeWinner->IsWon(BLACK, arrBoard) || m_pJudgeWinner->IsWon(WHITE, arrBoard))
	{
		return nScore;
	}

	int nBestScore = 0x7fffffff;  //初值是最大的的正数，要选出最小的分数
	QVector<QVector<int>> vecCandidator =  GenCandidator(arrBoard, BLACK);

	for(int i = 0; i< vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];        //待选的点
		arrBoard[vecPoint[0]][vecPoint[1]] = BLACK;     //人类下一个子
		int nScore = MaxSearch(arrBoard, deep-1, nBestScore<alpha?nBestScore:alpha, beta);     //预估机器下子的分数
		
		arrBoard[vecPoint[0]][vecPoint[1]] = NONE;
		if(nScore < nBestScore)
			nBestScore = nScore;
		//这是min层，需要在下一层中找到最小值，而nScore是目前得到的下一层的一个值，
		//beta是这一层目前得到的一个最大值，而这一层要选出一个最大值给上一层，如果目前的
		//对于这个节点，他要选一个最小值，那么选出的值肯定是小于等于nScore的，而这一层是要选个最大值
		//给上一层，因此肯定不会选这个节点了，故这个子节点的后面的子节点不要了。
		if(nScore<beta)
			break;
	}
	return nBestScore;
}

int ComputerMove::MaxSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta)
{
	int nScore = m_pEvalution->EvaluateBoard(arrBoard);

	if(deep <=0 || m_pJudgeWinner->IsWon(BLACK, arrBoard) || m_pJudgeWinner->IsWon(WHITE, arrBoard))
	{
		return nScore;
	}

	int nBestScore = 0x8fffffff;  //初值是最小的负数，要选出最大的分数
	QVector<QVector<int>> vecCandidator =  GenCandidator(arrBoard, WHITE);

	for(int i = 0; i< vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];        //待选的点
		arrBoard[vecPoint[0]][vecPoint[1]] = WHITE;     //机器下一个子
		int nScore = MinSearch(arrBoard, deep-1, alpha, nBestScore>beta?nBestScore:beta);     //预估人类下子的分数
		
		arrBoard[vecPoint[0]][vecPoint[1]] = NONE; 		
		if(nScore > nBestScore)
			nBestScore = nScore;
		//这是max层，要在子节点中找到一个max,目前子节点有个值是nScore,那么这个节点的值肯定就不会小于nScore了，
		//而上一层是min层，是要在这一层中找到一个min，而目前这一层的min是alpha，如果nScore大于alpha，那么肯定不会选择这个节点了
		//所以这个子节点后面的兄弟节点都不要了。
		if(nScore>alpha)
			break;
	}
	return nBestScore;
}
