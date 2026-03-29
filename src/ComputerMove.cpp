#include "ComputerMove.h"
#include <QRandomGenerator>
#include <algorithm>
#include <utility>
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

// 生成候选落子点：空位且八邻域至少有一枚棋子（无排序，轻量级）。
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
	// ── 优先级1：自己能直接赢就走 ──
	if (m_pEvalution->HasWinningMove(arrBoard, WHITE))
	{
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				if (arrBoard[i][j] != NONE)
					continue;
				arrBoard[i][j] = WHITE;
				bool won = m_pJudgeWinner->IsWon(WHITE, arrBoard);
				arrBoard[i][j] = NONE;
				if (won)
				{
					x = i;
					y = j;
					return;
				}
			}
		}
	}

	// ── 优先级2：对手有冲四/活四，必须立即封堵 ──
	if (m_pEvalution->HasWinningMove(arrBoard, BLACK))
	{
		// 遍历所有空位，找到消除对手必赢局面的位置。
		// 冲四：封堵开口端；活四：两端开口无法封堵，走正常搜索。
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				if (arrBoard[i][j] != NONE)
					continue;
				arrBoard[i][j] = WHITE;
				bool stillThreat = m_pEvalution->HasWinningMove(arrBoard, BLACK);
				arrBoard[i][j] = NONE;
				if (!stillThreat)
				{
					x = i;
					y = j;
					return;
				}
			}
		}
		// 活四无法封堵，继续正常搜索。
	}

	// ── 优先级3：对手有多个活三（双活三/跳活三），属于必杀局面 ──
	// CountOpenThrees 返回关键位置总数，>=4 意味着至少两个独立的活三，
	// 对手下一步可成活四（两个方向无法同时封堵）。
	const int oppKeys = m_pEvalution->CountOpenThrees(arrBoard, BLACK);
	if (oppKeys >= 4)
	{
		// 尝试找到一个位置同时消除最多的活三关键点。
		// 优先选择阻断对手活三数量最多的位置。
		int bestBlockCount = -1;
		int bestR = -1, bestC = -1;
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				if (arrBoard[i][j] != NONE)
					continue;
				arrBoard[i][j] = WHITE;
				int newOppKeys = m_pEvalution->CountOpenThrees(arrBoard, BLACK);
				arrBoard[i][j] = NONE;
				if (newOppKeys < 0) newOppKeys = 0;
				int blocked = oppKeys - newOppKeys;
				if (blocked > bestBlockCount)
				{
					bestBlockCount = blocked;
					bestR = i;
					bestC = j;
				}
			}
		}
		if (bestBlockCount > 0)
		{
			x = bestR;
			y = bestC;
			return;
		}
	}

	// ── 优先级4：自己能创建活三的进攻 ──
	const int myKeys = m_pEvalution->CountOpenThrees(arrBoard, WHITE);
	if (myKeys >= 4)
	{
		// 自己有双活三，直接走让活三数最多的位置。
		int bestAdd = -1;
		int bestR = -1, bestC = -1;
		for (int i = 0; i < 15; ++i)
		{
			for (int j = 0; j < 15; ++j)
			{
				if (arrBoard[i][j] != NONE)
					continue;
				arrBoard[i][j] = WHITE;
				int newMyKeys = m_pEvalution->CountOpenThrees(arrBoard, WHITE);
				arrBoard[i][j] = NONE;
				if (newMyKeys < 0) newMyKeys = 0;
				int added = newMyKeys - myKeys;
				if (added > bestAdd)
				{
					bestAdd = added;
					bestR = i;
					bestC = j;
				}
			}
		}
		if (bestAdd > 0)
		{
			x = bestR;
			y = bestC;
			return;
		}
	}

	QVector<QVector<int>> vecCandidator = GenCandidator(arrBoard, WHITE);

	// 评估排序：对每个候选点计算综合分值 = 己方增益 + 对手阻断分。
	// 先搜高分点可以让 alpha-beta 剪枝效率提升数倍。
	QVector<std::pair<int, QVector<int>>> scored;
	scored.reserve(vecCandidator.size());
	for (int i = 0; i < vecCandidator.size(); ++i)
	{
		const int r = vecCandidator[i][0];
		const int c = vecCandidator[i][1];
		// 己方落子增益：落WHITE后的评估分增量。
		const int attackScore = m_pEvalution->EvaluateMove(arrBoard, r, c, WHITE);
		// 对手阻断价值：落BLACK后的评估分增量（表示如果对手走这里有多大价值）。
		const int defenseScore = m_pEvalution->EvaluateMove(arrBoard, r, c, BLACK);
		// 综合分：进攻 + 防守（适当加权防守，因为忽视防守是当前AI最大弱点）。
		const int totalScore = attackScore + defenseScore * 2;
		scored.push_back({totalScore, vecCandidator[i]});
	}
	// 按综合分降序排列，高分优先搜索。
	std::sort(scored.begin(), scored.end(),
		[](const std::pair<int, QVector<int>> &a, const std::pair<int, QVector<int>> &b)
		{
			return a.first > b.first;
		});

	// 限制搜索宽度：只搜索前 N 个最高分候选点，避免在大量低价值点上浪费时间。
	// 动态调整：棋子少时搜更多点，棋子多时减少搜索宽度。
	const int searchWidth = (vecCandidator.size() <= 20) ? vecCandidator.size()
		: (vecCandidator.size() <= 40) ? 30 : 20;

	QVector<QVector<int>> vecBestPoints;
	int nBestScore = 0x8fffffff;
	const int searchCount = qMin(searchWidth, scored.size());
	for (int i = 0; i < searchCount; ++i)
	{
		QVector<int> vecPoint = scored[i].second;
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
