#include "ComputerMove.h"
#include <QRandomGenerator>
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
	// 简单随机策略：随机挑一个空位。
	QRandomGenerator *rng = QRandomGenerator::global();
	bool bOk = false;

	while (!bOk)
	{
		int randX = rng->bounded(15);
		int randY = rng->bounded(15);
		if (arrBoard[randX][randY] == NONE)
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
	for (int i = 0; i < 15; ++i)
	{
		for (int j = 0; j < 15; ++j)
		{
			Q_UNUSED(piece);
			if (arrBoard[i][j] == NONE)
			{
				// 候选点筛选：空位且八邻域至少有一枚棋子。
				if ((i - 1 >= 0 && j - 1 >= 0 && arrBoard[i - 1][j - 1] != NONE) ||
					(i - 1 >= 0 && arrBoard[i - 1][j] != NONE) ||
					(i - 1 >= 0 && j + 1 <= 14 && arrBoard[i - 1][j + 1] != NONE) ||
					(j - 1 >= 0 && arrBoard[i][j - 1] != NONE) ||
					(j + 1 <= 14 && arrBoard[i][j + 1] != NONE) ||
					(i + 1 <= 14 && j - 1 >= 0 && arrBoard[i + 1][j - 1] != NONE) ||
					(i + 1 <= 14 && arrBoard[i + 1][j] != NONE) ||
					(i + 1 <= 14 && j + 1 <= 14 && arrBoard[i + 1][j + 1] != NONE))
				{
					QVector<int> vec;
					vec.push_back(i);
					vec.push_back(j);
					vecSum.push_back(vec);
				}
			}
		}
	}
	return vecSum;
}

void ComputerMove::MaxMinSearch(ePiece (&arrBoard)[15][15], int deep)
{
	QVector<QVector<int>> vecCandidator = GenCandidator(arrBoard, WHITE);

	QVector<QVector<int>> vecBestPoints;
	int nBestScore = 0x8fffffff;
	for (int i = 0; i < vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];
		arrBoard[vecPoint[0]][vecPoint[1]] = WHITE;
		int nScore = MinSearch(arrBoard, deep - 1, 0x8fffffff, 0x7fffffff);

		if (nScore == nBestScore)
		{
			vecBestPoints.push_back(vecPoint);
		}
		if (nScore > nBestScore)
		{
			nBestScore = nScore;
			vecBestPoints.clear();
			vecBestPoints.push_back(vecPoint);
		}
		arrBoard[vecPoint[0]][vecPoint[1]] = NONE;
	}

	// 避免固定套路：从最高分候选中随机选择一个。
	int index = QRandomGenerator::global()->bounded(vecBestPoints.size());
	x = vecBestPoints[index][0];
	y = vecBestPoints[index][1];
}

int ComputerMove::MinSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta)
{
	int nScore = m_pEvalution->EvaluateBoard(arrBoard);
	if (deep <= 0 || m_pJudgeWinner->IsWon(BLACK, arrBoard) || m_pJudgeWinner->IsWon(WHITE, arrBoard))
	{
		return nScore;
	}

	int nBestScore = 0x7fffffff;
	QVector<QVector<int>> vecCandidator = GenCandidator(arrBoard, BLACK);

	for (int i = 0; i < vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];
		arrBoard[vecPoint[0]][vecPoint[1]] = BLACK;
		int nScore = MaxSearch(arrBoard, deep - 1, nBestScore < alpha ? nBestScore : alpha, beta);

		arrBoard[vecPoint[0]][vecPoint[1]] = NONE;
		if (nScore < nBestScore)
			nBestScore = nScore;

		// alpha-beta 剪枝：当前分支已经不可能更优，直接截断。
		if (nScore < beta)
			break;
	}
	return nBestScore;
}

int ComputerMove::MaxSearch(ePiece (&arrBoard)[15][15], int deep, int alpha, int beta)
{
	int nScore = m_pEvalution->EvaluateBoard(arrBoard);

	if (deep <= 0 || m_pJudgeWinner->IsWon(BLACK, arrBoard) || m_pJudgeWinner->IsWon(WHITE, arrBoard))
	{
		return nScore;
	}

	int nBestScore = 0x8fffffff;
	QVector<QVector<int>> vecCandidator = GenCandidator(arrBoard, WHITE);

	for (int i = 0; i < vecCandidator.size(); ++i)
	{
		QVector<int> vecPoint = vecCandidator[i];
		arrBoard[vecPoint[0]][vecPoint[1]] = WHITE;
		int nScore = MinSearch(arrBoard, deep - 1, alpha, nBestScore > beta ? nBestScore : beta);

		arrBoard[vecPoint[0]][vecPoint[1]] = NONE;
		if (nScore > nBestScore)
			nBestScore = nScore;

		// alpha-beta 剪枝：当前分支已可确定不会被上层选中。
		if (nScore > alpha)
			break;
	}
	return nBestScore;
}
