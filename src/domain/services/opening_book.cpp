#include "opening_book.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

namespace game_core {

// 将棋盘转为落子序列字符串作为查表键。
// 格式："r0,c0;r1,c1;..."，按 moveIndex 升序排列。
QString OpeningBook::boardKey(const GameBoard &board) const
{
    // 收集所有有棋子的位置及其落子顺序
    struct MoveInfo { int row, col, idx; };
    QVector<MoveInfo> moves;

    const auto &arr = board.board();
    for (int r = 0; r < GameBoard::BOARD_SIZE; ++r) {
        for (int c = 0; c < GameBoard::BOARD_SIZE; ++c) {
            if (arr[r][c] != Piece::None) {
                // moveIndex: 按棋子数量估算，无法精确还原顺序
                // 用行列编码作为稳定键
                moves.push_back({r, c, r * GameBoard::BOARD_SIZE + c});
            }
        }
    }

    // 按行列扫描顺序编码（开局库条目按相同顺序存储）
    std::sort(moves.begin(), moves.end(),
        [](const MoveInfo &a, const MoveInfo &b) { return a.idx < b.idx; });

    QString key;
    for (const auto &m : moves) {
        key += QString("%1,%2;").arg(m.row).arg(m.col);
    }
    return key;
}

int OpeningBook::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString content = in.readAll();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray())
        return -1;

    const QJsonArray entries = doc.array();
    int loaded = 0;

    for (const QJsonValue &val : entries) {
        if (!val.isObject()) continue;
        const QJsonObject obj = val.toObject();

        // 解析 "moves" 数组（已落子的局面）
        if (!obj.contains("moves") || !obj.contains("reply")) continue;

        const QJsonArray movesArr = obj["moves"].toArray();
        const QJsonArray replyArr = obj["reply"].toArray();
        if (replyArr.size() != 2) continue;

        const int replyRow = replyArr[0].toInt(-1);
        const int replyCol = replyArr[1].toInt(-1);
        if (replyRow < 0 || replyRow >= 15 || replyCol < 0 || replyCol >= 15) continue;

        // 构造局面键：用 moves 数组内容排序后拼接
        QString key;
        // moves 数组是 [[r,c], [r,c], ...] 按落子顺序排列
        // 为保证查表稳定，按行列数值排序
        QVector<QPair<int,int>> boardMoves;
        for (const QJsonValue &mv : movesArr) {
            const QJsonArray pos = mv.toArray();
            if (pos.size() != 2) continue;
            boardMoves.push_back({pos[0].toInt(), pos[1].toInt()});
        }
        std::sort(boardMoves.begin(), boardMoves.end(),
            [](const QPair<int,int> &a, const QPair<int,int> &b) {
                return a.first * 15 + a.second < b.first * 15 + b.second;
            });
        for (const auto &p : boardMoves)
            key += QString("%1,%2;").arg(p.first).arg(p.second);

        m_table[key].append(Position(replyRow, replyCol));
        ++loaded;
    }

    m_loaded = (loaded > 0);
    return loaded;
}

Position OpeningBook::lookup(const GameBoard &board, Piece /*piece*/) const
{
    ++m_queryCount;

    // 只在开局阶段（<=10手）使用开局库
    if (board.moveCount() > 10) {
        return Position::Invalid();
    }

    const QString key = boardKey(board);
    auto it = m_table.find(key);
    if (it == m_table.end() || it->isEmpty()) {
        return Position::Invalid();
    }

    ++m_hitCount;

    // 从推荐列表中随机选一个（增加多样性）
    m_seed = m_seed * 1664525u + 1013904223u;
    const int idx = static_cast<int>(m_seed % static_cast<unsigned>(it->size()));
    return (*it)[idx];
}

double OpeningBook::hitRate() const
{
    if (m_queryCount == 0) return 0.0;
    return static_cast<double>(m_hitCount) / m_queryCount;
}

} // namespace game_core
