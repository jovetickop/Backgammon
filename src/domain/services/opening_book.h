#pragma once

#include <QString>
#include <QHash>
#include <QVector>
#include "../aggregates/game_board.h"
#include "../values/piece.h"

namespace game_core {

// 开局库条目：棋盘局面哈希 -> 推荐落点列表。
struct OpeningEntry {
    QVector<Position> moves;  // 推荐落点（可有多个，随机选一个增加多样性）
};

// 开局库：从 JSON 文件加载，AI 开局阶段优先查表走棋。
// JSON 格式：
// [
//   { "moves": [[r0,c0],[r1,c1],...], "reply": [row, col] }
// ]
// moves 表示局面已有落子序列，reply 表示推荐回应。
class OpeningBook {
public:
    OpeningBook() = default;

    // 从 JSON 文件加载开局库。
    // filePath: JSON 文件路径。
    // 返回：加载成功的条目数量，-1 表示文件不存在或格式错误。
    int load(const QString &filePath);

    // 查询当前局面是否有开局库推荐落点。
    // board: 当前棋盘。
    // piece: 将要落子的棋子颜色。
    // 返回：推荐位置；若未命中返回 Position::Invalid()。
    Position lookup(const GameBoard &board, Piece piece) const;

    // 是否已加载。
    bool isLoaded() const { return m_loaded; }

    // 获取总查询次数。
    int queryCount() const { return m_queryCount; }

    // 获取命中次数。
    int hitCount() const { return m_hitCount; }

    // 命中率（0.0 ~ 1.0）。
    double hitRate() const;

private:
    // 将棋盘状态编码为唯一键（落子序列的字符串表示）。
    // 由于五子棋落子顺序对局面有意义，使用落子序列哈希。
    QString boardKey(const GameBoard &board) const;

    // 键 -> 推荐落点列表
    QHash<QString, QVector<Position>> m_table;

    bool m_loaded = false;
    mutable int m_queryCount = 0;
    mutable int m_hitCount   = 0;

    // 随机数引擎（用于从多个推荐落点中随机选一个）
    mutable unsigned int m_seed = 42;
};

} // namespace game_core
