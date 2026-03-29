#include "Evaluation.h"

Evaluation::Evaluation(void)
{
}

Evaluation::~Evaluation(void)
{
}

int Evaluation::EvaluateBoard(ePiece (&arrBoard)[15][15])
{
	// 返回白方分 - 黑方分，分值越大越有利于白方（电脑）。
	QVector<QVector<ePiece>> vecSum = CutBoard(arrBoard);
	return Score(arrBoard, WHITE, vecSum) - Score(arrBoard, BLACK, vecSum);
}

QVector<QVector<ePiece>> Evaluation::CutBoard(ePiece (&arrBoard)[15][15])
{
	// 将棋盘按横、纵、两条斜线方向切分为一组一维线段。
	QVector<QVector<ePiece>> vecSum;

	// 横向切分（每行一条线段）。
	for (int j = 0; j < 15; j++)
	{
		QVector<ePiece> vec;
		for (int i = 0; i < 15; i++)
			vec.push_back(arrBoard[i][j]);
		vecSum.push_back(vec);
	}

	// 纵向切分（每列一条线段）。
	for (int j = 0; j < 15; j++)
	{
		QVector<ePiece> vec;
		for (int i = 0; i < 15; i++)
			vec.push_back(arrBoard[j][i]);
		vecSum.push_back(vec);
	}

	// 左上到右下对角线切分。
	for (int j = 14; j >= 0; --j)
	{
		QVector<ePiece> vec;
		int m = 0, n = j;
		while (n < 15)
		{
			vec.push_back(arrBoard[m][n]);
			++n; ++m;
		}
		vecSum.push_back(vec);
	}
	for (int i = 1; i < 15; ++i)
	{
		QVector<ePiece> vec;
		int m = i, n = 0;
		while (m < 15)
		{
			vec.push_back(arrBoard[m][n]);
			++n; ++m;
		}
		vecSum.push_back(vec);
	}

	// 右上到左下对角线切分。
	for (int j = 14; j >= 0; --j)
	{
		QVector<ePiece> vec;
		int m = 14, n = j;
		while (n < 15)
		{
			vec.push_back(arrBoard[m][n]);
			++n; --m;
		}
		vecSum.push_back(vec);
	}
	for (int i = 13; i >= 0; --i)
	{
		QVector<ePiece> vec;
		int m = i, n = 0;
		while (m >= 0)
		{
			vec.push_back(arrBoard[m][n]);
			++n; --m;
		}
		vecSum.push_back(vec);
	}

	return vecSum;
}

int Evaluation::Score(ePiece (&arrBoard)[15][15], ePiece piece, QVector<QVector<ePiece>> &vecSum)
{
	Q_UNUSED(arrBoard);
	int totalScore = 0;

	for (int lineIdx = 0; lineIdx < vecSum.size(); ++lineIdx)
	{
		const QVector<ePiece> &line = vecSum[lineIdx];
		const int len = line.size();

		// 用滑窗扫描：在每个位置尝试 1~5 子窗口，
		// 只在窗口起始位置计分，避免同一段连续棋子被重复计算。
		for (int pos = 0; pos < len; ++pos)
		{
			if (line[pos] != piece)
				continue;

			int bestPatternScore = 0;

			// 检查从 pos 开始的连续棋子数量（最大 5）。
			int consecutive = 0;
			for (int k = pos; k < len && line[k] == piece && consecutive < 5; ++k)
				++consecutive;

			if (consecutive >= 5)
			{
				// 五连：无论封堵情况如何都算赢。
				bestPatternScore = FIVE;
			}
			else if (consecutive == 4)
			{
				// 四连：判断两端封堵情况。
				bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == NONE);
				bool rightOpen = (pos + 4 <= len - 1 && line[pos + 4] == NONE);
				if (leftOpen && rightOpen)
					bestPatternScore = OPEN_FOUR;       // 活四：两端均空，必胜。
				else if (leftOpen || rightOpen)
					bestPatternScore = CLOSE_FOUR;      // 冲四：一端空，可赢。
				else
					bestPatternScore = 0;                // 死四：两端全堵，无价值。
			}
			else if (consecutive == 3)
			{
				bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == NONE);
				bool rightOpen = (pos + 3 <= len - 1 && line[pos + 3] == NONE);
				if (leftOpen && rightOpen)
					bestPatternScore = OPEN_THREE;       // 活三：两端均空。
				else if (leftOpen || rightOpen)
					bestPatternScore = CLOSE_THREE;      // 眠三：一端空。
				else
					bestPatternScore = 0;                // 死三：两端全堵。
			}
			else if (consecutive == 2)
			{
				bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == NONE);
				bool rightOpen = (pos + 2 <= len - 1 && line[pos + 2] == NONE);
				if (leftOpen && rightOpen)
					bestPatternScore = OPEN_TWO;         // 活二。
				else if (leftOpen || rightOpen)
					bestPatternScore = CLOSE_TWO;        // 眠二。
				else
					bestPatternScore = 0;                // 死二。
			}
			else if (consecutive == 1)
			{
				bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == NONE);
				bool rightOpen = (pos + 1 <= len - 1 && line[pos + 1] == NONE);
				if (leftOpen && rightOpen)
					bestPatternScore = OPEN_ONE;         // 活一。
				else if (leftOpen || rightOpen)
					bestPatternScore = CLOSE_ONE;        // 眠一。
				else
					bestPatternScore = 0;                // 完全封堵的单子无价值。
			}

			totalScore += bestPatternScore;

			// 跳过这段连续棋子，避免重复计分。
			pos += consecutive - 1;
		}
	}

	return totalScore;
}

// 检测指定棋子是否在某条线段上有四连且至少一端为空（可一步五连）。
static bool LineHasWinningMove(const ePiece line[], int len, ePiece piece)
{
	for (int pos = 0; pos < len; ++pos)
	{
		if (line[pos] != piece)
			continue;

		// 计算从 pos 开始的连续同色棋子数。
		int consecutive = 0;
		for (int k = pos; k < len && line[k] == piece && consecutive < 5; ++k)
			++consecutive;

		// 只关注四连：活四（两端空）或冲四（一端空）。
		if (consecutive == 4)
		{
			bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == NONE);
			bool rightOpen = (pos + 4 <= len - 1 && line[pos + 4] == NONE);
			if (leftOpen || rightOpen)
				return true;
		}

		// 跳过已检查的连续段。
		pos += consecutive - 1;
	}
	return false;
}

// 在滑窗大小为5的范围内检查某个棋子是否出现。
static bool WindowContains(const ePiece line[], int start, int end, ePiece piece)
{
	for (int i = start; i < end; ++i)
	{
		if (line[i] == piece)
			return true;
	}
	return false;
}

// 检测一条线段上的活三（连续三子两端开口）或跳活三（三子中有间隔但填充后可成活四）。
// 返回该线段上活三的"关键进攻位置"数量（每个活三的开口端 + 间隔位置）。
// 返回值 -1 表示该线段上无活三。
static int LineOpenThreeKeys(const ePiece line[], int len, ePiece piece, const ePiece opponent, QVector<QVector<int>> &allKeys)
{
	int found = 0;
	for (int pos = 0; pos < len - 4; ++pos)
	{
		// 在 pos~pos+4 的5子窗口中统计己方棋子数和对手棋子数。
		int own = 0, opp = 0;
		int ownPos[5]; // 己方棋子在窗口内的索引
		for (int k = 0; k < 5; ++k)
		{
			if (line[pos + k] == piece)
			{
				ownPos[own] = pos + k;
				++own;
			}
			else if (line[pos + k] == opponent)
			{
				++opp;
			}
		}
		// 活三条件：恰好3己方、0对手，且有2个空位可作为开口。
		if (own != 3 || opp != 0)
			continue;

		// 收集关键位置（空位）。
		QVector<int> keys;
		for (int k = 0; k < 5; ++k)
		{
			if (line[pos + k] == NONE)
				keys.push_back(pos + k);
		}

		// 窗口边界外再确认两端至少各有一个空位（否则不是真正的活三）。
		bool leftOk = (pos > 0 && line[pos - 1] == NONE) || keys.contains(pos);
		bool rightOk = (pos + 4 < len - 1 && line[pos + 5] == NONE) || keys.contains(pos + 4);

		// 只要窗口外端有空位或者端点本身是关键位置，就形成活三。
		// 简化判断：窗口内3子+2空=活三模式，关键位置就是这2个空位。
		// 如果端点也在边界处且被堵住，该活三价值降低，但仍需防守。
		if (leftOk || rightOk)
		{
			allKeys.push_back(keys);
			found += keys.size(); // 关键位置数（通常为2）
		}
	}
	return found;
}

int Evaluation::CountOpenThrees(ePiece (&arrBoard)[15][15], ePiece piece)
{
	ePiece line[15];
	const ePiece opponent = (piece == BLACK) ? WHITE : BLACK;
	QVector<QVector<int>> allKeys;
	int totalKeys = 0;

	// 横向扫描。
	for (int j = 0; j < 15; ++j)
	{
		for (int i = 0; i < 15; ++i)
			line[i] = arrBoard[i][j];
		totalKeys += LineOpenThreeKeys(line, 15, piece, opponent, allKeys);
	}
	// 纵向扫描。
	for (int j = 0; j < 15; ++j)
	{
		for (int i = 0; i < 15; ++i)
			line[i] = arrBoard[j][i];
		totalKeys += LineOpenThreeKeys(line, 15, piece, opponent, allKeys);
	}
	// 左上到右下对角线扫描。
	for (int j = 14; j >= 0; --j)
	{
		int len = 15 - j;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[k][j + k];
		if (len >= 5)
			totalKeys += LineOpenThreeKeys(line, len, piece, opponent, allKeys);
	}
	for (int i = 1; i < 15; ++i)
	{
		int len = 15 - i;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[i + k][k];
		if (len >= 5)
			totalKeys += LineOpenThreeKeys(line, len, piece, opponent, allKeys);
	}
	// 右上到左下对角线扫描。
	for (int j = 14; j >= 0; --j)
	{
		int len = 15 - j;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[14 - k][j + k];
		if (len >= 5)
			totalKeys += LineOpenThreeKeys(line, len, piece, opponent, allKeys);
	}
	for (int i = 13; i >= 0; --i)
	{
		int len = i + 1;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[i - k][k];
		if (len >= 5)
			totalKeys += LineOpenThreeKeys(line, len, piece, opponent, allKeys);
	}

	return (totalKeys > 0) ? totalKeys : -1;
}

int Evaluation::EvaluateMove(ePiece (&arrBoard)[15][15], int row, int col, ePiece piece)
{
	// 临时落子，评估该位置对 piece 的增益分值。
	arrBoard[row][col] = piece;
	const int score = EvaluateBoard(arrBoard);
	arrBoard[row][col] = NONE;
	return score;
}

bool Evaluation::HasWinningMove(ePiece (&arrBoard)[15][15], ePiece piece)
{
	ePiece line[15];

	// 横向扫描（每行一条线段）。
	for (int j = 0; j < 15; ++j)
	{
		for (int i = 0; i < 15; ++i)
			line[i] = arrBoard[i][j];
		if (LineHasWinningMove(line, 15, piece))
			return true;
	}

	// 纵向扫描（每列一条线段）。
	for (int j = 0; j < 15; ++j)
	{
		for (int i = 0; i < 15; ++i)
			line[i] = arrBoard[j][i];
		if (LineHasWinningMove(line, 15, piece))
			return true;
	}

	// 左上到右下对角线扫描。
	for (int j = 14; j >= 0; --j)
	{
		int len = 15 - j;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[k][j + k];
		if (LineHasWinningMove(line, len, piece))
			return true;
	}
	for (int i = 1; i < 15; ++i)
	{
		int len = 15 - i;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[i + k][k];
		if (LineHasWinningMove(line, len, piece))
			return true;
	}

	// 右上到左下对角线扫描。
	for (int j = 14; j >= 0; --j)
	{
		int len = 15 - j;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[14 - k][j + k];
		if (LineHasWinningMove(line, len, piece))
			return true;
	}
	for (int i = 13; i >= 0; --i)
	{
		int len = i + 1;
		for (int k = 0; k < len; ++k)
			line[k] = arrBoard[i - k][k];
		if (LineHasWinningMove(line, len, piece))
			return true;
	}

	return false;
}

// 以下函数保留用于 ComputerMove 搜索逻辑中的局面评估，保持 AI 决策兼容性。
// 注意：这些旧函数可能存在重复计分问题，但 AI 搜索是相对比较所以影响不大。

bool Evaluation::IsOpenOne(QVector<ePiece> vec, int index, ePiece piece)
{
	if (index - 1 >= 0 && index + 1 <= vec.size() - 1)
	{
		if (vec[index - 1] == NONE && vec[index + 1] == NONE && vec[index] == piece)
			return true;
	}
	return false;
}

bool Evaluation::IsOpenTwo(QVector<ePiece> vec, int index, ePiece piece)
{
	if (index - 1 >= 0 && index + 2 <= vec.size() - 1)
	{
		if (vec[index - 1] == NONE && vec[index + 2] == NONE && vec[index] == piece && vec[index + 1] == piece)
			return true;
	}
	return false;
}

bool Evaluation::IsOpenThree(QVector<ePiece> vec, int index, ePiece piece)
{
	if (index - 1 >= 0 && index + 3 <= vec.size() - 1)
	{
		if (vec[index - 1] == NONE && vec[index + 3] == NONE && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece)
			return true;
	}
	return false;
}

bool Evaluation::IsOpenFour(QVector<ePiece> vec, int index, ePiece piece)
{
	if (index - 1 >= 0 && index + 4 <= vec.size() - 1)
	{
		if (vec[index - 1] == NONE && vec[index + 4] == NONE && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece && vec[index + 3] == piece)
			return true;
	}
	return false;
}

bool Evaluation::IsFive(QVector<ePiece> vec, int index, ePiece piece)
{
	if (index + 4 <= vec.size() - 1)
	{
		if (vec[index + 4] == piece && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece && vec[index + 3] == piece)
			return true;
	}
	return false;
}

bool Evaluation::IsCloseOne(QVector<ePiece> vec, int index, ePiece piece)
{
	if (vec.size() > 1 && index == 0 && vec[index] == piece && vec[index + 1] == NONE)
		return true;
	if (vec.size() > 1 && index == vec.size() - 1 && vec[index] == piece && vec[index - 1] == NONE)
		return true;
	if (vec.size() > 2 && index - 1 >= 0 && index + 1 <= vec.size() - 1 && vec[index] == piece)
	{
		if ((vec[index - 1] != piece && vec[index - 1] != NONE && vec[index + 1] == NONE) ||
			(vec[index + 1] != piece && vec[index + 1] != NONE && vec[index - 1] == NONE))
			return true;
	}
	return false;
}

bool Evaluation::IsCloseTwO(QVector<ePiece> vec, int index, ePiece piece)
{
	if (vec.size() > 2 && index == 0 && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == NONE)
		return true;
	if (vec.size() > 2 && index == vec.size() - 1 && vec[index] == piece && vec[index - 1] == piece && vec[index - 2] == NONE)
		return true;
	if (vec.size() > 3 && index - 1 >= 0 && index + 2 <= vec.size() - 1 && vec[index] == piece && vec[index + 1] == piece)
	{
		if ((vec[index - 1] != piece && vec[index - 1] != NONE && vec[index + 2] == NONE) ||
			(vec[index + 2] != piece && vec[index + 2] != NONE && vec[index - 1] == NONE))
			return true;
	}
	return false;
}

bool Evaluation::IsCloseThree(QVector<ePiece> vec, int index, ePiece piece)
{
	if (vec.size() > 3 && index == 0 && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece && vec[index + 3] == NONE)
		return true;
	if (vec.size() > 3 && index == vec.size() - 1 && vec[index] == piece && vec[index - 1] == piece && vec[index - 2] == piece && vec[index - 3] == NONE)
		return true;
	if (vec.size() > 4 && index - 1 >= 0 && index + 3 <= vec.size() - 1 && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece)
	{
		if ((vec[index - 1] != piece && vec[index - 1] != NONE && vec[index + 3] == NONE) ||
			(vec[index + 3] != piece && vec[index + 3] != NONE && vec[index - 1] == NONE))
			return true;
	}
	return false;
}

bool Evaluation::IsCloseFour(QVector<ePiece> vec, int index, ePiece piece)
{
	if (vec.size() > 4 && index == 0 && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece && vec[index + 3] == piece && vec[index + 4] == NONE)
		return true;
	if (vec.size() > 4 && index == vec.size() - 1 && vec[index] == piece && vec[index - 1] == piece && vec[index - 2] == piece && vec[index - 3] == piece && vec[index - 4] == NONE)
		return true;
	if (vec.size() > 5 && index - 1 >= 0 && index + 4 <= vec.size() - 1 && vec[index] == piece && vec[index + 1] == piece && vec[index + 2] == piece && vec[index + 3] == piece)
	{
		if ((vec[index - 1] != piece && vec[index - 1] != NONE && vec[index + 4] == NONE) ||
			(vec[index + 4] != piece && vec[index + 4] != NONE && vec[index - 1] == NONE))
			return true;
	}
	return false;
}
