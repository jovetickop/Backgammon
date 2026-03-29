#include "board_evaluator.h"
#include "Evaluation.h"
#include "types.h"
#include <algorithm>
#include <utility>
#include <vector>
#include <cstring>

namespace game_core {

// 转换为内部Piece类型
static Piece convertPiece(ePiece p) {
    if (p == WHITE) return Piece::White;
    if (p == BLACK) return Piece::Black;
    return Piece::None;
}

// 转换为ePiece
static ePiece toEPiece(Piece p) {
    if (p == Piece::White) return WHITE;
    if (p == Piece::Black) return BLACK;
    return NONE;
}

int BoardEvaluator::evaluate(const GameBoard& board) {
    // 将GameBoard转换为ePiece数组供原有函数使用
    ePiece arrBoard[15][15];
    const auto& src = board.board();
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            arrBoard[i][j] = toEPiece(src[i][j]);
        }
    }

    // 调用原有评估函数
    Evaluation eval;
    return eval.EvaluateBoard(arrBoard);
}

int BoardEvaluator::evaluateMove(const GameBoard& board, const Position& pos, Piece piece) {
    // 临时落子，评估该位置对 piece 的增益分值
    GameBoard temp = board;  // 复制棋盘
    temp.placePiece(pos, piece);
    return evaluate(temp) - evaluate(board);
}

std::vector<CandidateMove> BoardEvaluator::generateCandidates(const GameBoard& board, Piece piece) {
    // 生成候选落子点：靠近已有棋子的位置
    std::vector<CandidateMove> candidates;

    // 记录已有棋子的位置
    bool hasPiece[15][15] = {};
    int minRow = 15, maxRow = -1, minCol = 15, maxCol = -1;

    const auto& src = board.board();
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            if (src[i][j] != Piece::None) {
                hasPiece[i][j] = true;
                minRow = std::min(minRow, i);
                maxRow = std::max(maxRow, i);
                minCol = std::min(minCol, j);
                maxCol = std::max(maxCol, j);
            }
        }
    }

    // 如果棋盘为空，返回中心点
    if (minRow > maxRow) {
        candidates.emplace_back(Position(7, 7), 0);
        return candidates;
    }

    // 扩展搜索范围
    int searchMinRow = std::max(0, minRow - 2);
    int searchMaxRow = std::min(14, maxRow + 2);
    int searchMinCol = std::max(0, minCol - 2);
    int searchMaxCol = std::min(14, maxCol + 2);

    // 收集空位
    for (int i = searchMinRow; i <= searchMaxRow; ++i) {
        for (int j = searchMinCol; j <= searchMaxCol; ++j) {
            if (!hasPiece[i][j]) {
                // 检查是否靠近已有棋子
                bool nearPiece = false;
                for (int di = -2; di <= 2 && !nearPiece; ++di) {
                    for (int dj = -2; dj <= 2 && !nearPiece; ++dj) {
                        int ni = i + di, nj = j + dj;
                        if (ni >= 0 && ni < 15 && nj >= 0 && nj < 15) {
                            if (hasPiece[ni][nj]) {
                                nearPiece = true;
                            }
                        }
                    }
                }

                if (nearPiece) {
                    Position pos(i, j);
                    int score = evaluateMove(board, pos, piece);
                    candidates.emplace_back(pos, score);
                }
            }
        }
    }

    // 按分数降序排列
    std::sort(candidates.begin(), candidates.end(),
              [](const CandidateMove& a, const CandidateMove& b) {
                  return a.score > b.score;
              });

    return candidates;
}

bool BoardEvaluator::hasWinningMove(const GameBoard& board, Piece piece) {
    // 将GameBoard转换为ePiece数组供原有函数使用
    ePiece arrBoard[15][15];
    const auto& src = board.board();
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            arrBoard[i][j] = toEPiece(src[i][j]);
        }
    }

    Evaluation eval;
    return eval.HasWinningMove(arrBoard, toEPiece(piece));
}

int BoardEvaluator::countOpenThrees(const GameBoard& board, Piece piece) {
    // 将GameBoard转换为ePiece数组供原有函数使用
    ePiece arrBoard[15][15];
    const auto& src = board.board();
    for (int i = 0; i < 15; ++i) {
        for (int j = 0; j < 15; ++j) {
            arrBoard[i][j] = toEPiece(src[i][j]);
        }
    }

    Evaluation eval;
    int result = eval.CountOpenThrees(arrBoard, toEPiece(piece));
    return result > 0 ? result : 0;
}

std::vector<std::vector<Piece>> BoardEvaluator::cutBoard(const GameBoard& board) {
    // 实现参考原有Evaluation::CutBoard
    std::vector<std::vector<Piece>> vecSum;
    const auto& arr = board.board();

    // 横向切分
    for (int j = 0; j < 15; ++j) {
        std::vector<Piece> vec;
        for (int i = 0; i < 15; ++i) {
            vec.push_back(arr[i][j]);
        }
        vecSum.push_back(vec);
    }

    // 纵向切分
    for (int j = 0; j < 15; ++j) {
        std::vector<Piece> vec;
        for (int i = 0; i < 15; ++i) {
            vec.push_back(arr[j][i]);
        }
        vecSum.push_back(vec);
    }

    // 两条对角线方向省略（完整实现参考原有代码）

    return vecSum;
}

int BoardEvaluator::score(const GameBoard& board, Piece piece, std::vector<std::vector<Piece>>& vecSum) {
    // 实现参考原有Evaluation::Score
    Q_UNUSED(board);
    int totalScore = 0;

    for (const auto& line : vecSum) {
        const int len = static_cast<int>(line.size());

        for (int pos = 0; pos < len; ++pos) {
            if (line[pos] != piece)
                continue;

            int bestPatternScore = 0;

            // 检查连续棋子数量
            int consecutive = 0;
            for (int k = pos; k < len && line[k] == piece && consecutive < 5; ++k)
                ++consecutive;

            if (consecutive >= 5) {
                bestPatternScore = static_cast<int>(ScoreType::Five);
            } else if (consecutive == 4) {
                bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == Piece::None);
                bool rightOpen = (pos + 4 <= len - 1 && line[pos + 4] == Piece::None);
                if (leftOpen && rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::OpenFour);
                else if (leftOpen || rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::CloseFour);
            } else if (consecutive == 3) {
                bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == Piece::None);
                bool rightOpen = (pos + 3 <= len - 1 && line[pos + 3] == Piece::None);
                if (leftOpen && rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::OpenThree);
                else if (leftOpen || rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::CloseThree);
            } else if (consecutive == 2) {
                bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == Piece::None);
                bool rightOpen = (pos + 2 <= len - 1 && line[pos + 2] == Piece::None);
                if (leftOpen && rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::OpenTwo);
                else if (leftOpen || rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::CloseTwo);
            } else if (consecutive == 1) {
                bool leftOpen = (pos - 1 >= 0 && line[pos - 1] == Piece::None);
                bool rightOpen = (pos + 1 <= len - 1 && line[pos + 1] == Piece::None);
                if (leftOpen || rightOpen)
                    bestPatternScore = static_cast<int>(ScoreType::OpenOne);
            }

            totalScore += bestPatternScore;
            pos += consecutive - 1;
        }
    }

    return totalScore;
}

bool BoardEvaluator::isOpenOne(const std::vector<Piece>& line, int index, Piece piece) {
    if (index - 1 >= 0 && index + 1 <= static_cast<int>(line.size()) - 1) {
        return line[index - 1] == Piece::None && line[index + 1] == Piece::None && line[index] == piece;
    }
    return false;
}

bool BoardEvaluator::isOpenTwo(const std::vector<Piece>& line, int index, Piece piece) {
    if (index - 1 >= 0 && index + 2 <= static_cast<int>(line.size()) - 1) {
        return line[index - 1] == Piece::None && line[index + 2] == Piece::None &&
               line[index] == piece && line[index + 1] == piece;
    }
    return false;
}

bool BoardEvaluator::isOpenThree(const std::vector<Piece>& line, int index, Piece piece) {
    if (index - 1 >= 0 && index + 3 <= static_cast<int>(line.size()) - 1) {
        return line[index - 1] == Piece::None && line[index + 3] == Piece::None &&
               line[index] == piece && line[index + 1] == piece && line[index + 2] == piece;
    }
    return false;
}

bool BoardEvaluator::isOpenFour(const std::vector<Piece>& line, int index, Piece piece) {
    if (index - 1 >= 0 && index + 4 <= static_cast<int>(line.size()) - 1) {
        return line[index - 1] == Piece::None && line[index + 4] == Piece::None &&
               line[index] == piece && line[index + 1] == piece && line[index + 2] == piece && line[index + 3] == piece;
    }
    return false;
}

bool BoardEvaluator::isFive(const std::vector<Piece>& line, int index, Piece piece) {
    if (index + 4 <= static_cast<int>(line.size()) - 1) {
        return line[index] == piece && line[index + 1] == piece && line[index + 2] == piece &&
               line[index + 3] == piece && line[index + 4] == piece;
    }
    return false;
}

bool BoardEvaluator::isCloseOne(const std::vector<Piece>& line, int index, Piece piece) {
    int len = static_cast<int>(line.size());
    if (len > 1 && index == 0 && line[index] == piece && line[index + 1] == Piece::None)
        return true;
    if (len > 1 && index == len - 1 && line[index] == piece && line[index - 1] == Piece::None)
        return true;
    if (len > 2 && index - 1 >= 0 && index + 1 <= len - 1 && line[index] == piece) {
        if ((line[index - 1] != piece && line[index - 1] != Piece::None && line[index + 1] == Piece::None) ||
            (line[index + 1] != piece && line[index + 1] != Piece::None && line[index - 1] == Piece::None))
            return true;
    }
    return false;
}

bool BoardEvaluator::isCloseTwo(const std::vector<Piece>& line, int index, Piece piece) {
    int len = static_cast<int>(line.size());
    if (len > 2 && index == 0 && line[index] == piece && line[index + 1] == piece && line[index + 2] == Piece::None)
        return true;
    if (len > 2 && index == len - 1 && line[index] == piece && line[index - 1] == piece && line[index - 2] == Piece::None)
        return true;
    if (len > 3 && index - 1 >= 0 && index + 2 <= len - 1 && line[index] == piece && line[index + 1] == piece) {
        if ((line[index - 1] != piece && line[index - 1] != Piece::None && line[index + 2] == Piece::None) ||
            (line[index + 2] != piece && line[index + 2] != Piece::None && line[index - 1] == Piece::None))
            return true;
    }
    return false;
}

bool BoardEvaluator::isCloseThree(const std::vector<Piece>& line, int index, Piece piece) {
    int len = static_cast<int>(line.size());
    if (len > 3 && index == 0 && line[index] == piece && line[index + 1] == piece &&
        line[index + 2] == piece && line[index + 3] == Piece::None)
        return true;
    if (len > 3 && index == len - 1 && line[index] == piece && line[index - 1] == piece &&
        line[index - 2] == piece && line[index - 3] == Piece::None)
        return true;
    if (len > 4 && index - 1 >= 0 && index + 3 <= len - 1 && line[index] == piece &&
        line[index + 1] == piece && line[index + 2] == piece) {
        if ((line[index - 1] != piece && line[index - 1] != Piece::None && line[index + 3] == Piece::None) ||
            (line[index + 3] != piece && line[index + 3] != Piece::None && line[index - 1] == Piece::None))
            return true;
    }
    return false;
}

bool BoardEvaluator::isCloseFour(const std::vector<Piece>& line, int index, Piece piece) {
    int len = static_cast<int>(line.size());
    if (len > 4 && index == 0 && line[index] == piece && line[index + 1] == piece &&
        line[index + 2] == piece && line[index + 3] == piece && line[index + 4] == Piece::None)
        return true;
    if (len > 4 && index == len - 1 && line[index] == piece && line[index - 1] == piece &&
        line[index - 2] == piece && line[index - 3] == piece && line[index - 4] == Piece::None)
        return true;
    if (len > 5 && index - 1 >= 0 && index + 4 <= len - 1 && line[index] == piece &&
        line[index + 1] == piece && line[index + 2] == piece && line[index + 3] == piece) {
        if ((line[index - 1] != piece && line[index - 1] != Piece::None && line[index + 4] == Piece::None) ||
            (line[index + 4] != piece && line[index + 4] != Piece::None && line[index - 1] == Piece::None))
            return true;
    }
    return false;
}

} // namespace game_core
