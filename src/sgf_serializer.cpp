#include "sgf_serializer.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include <QTextCodec>

// 将索引转为 SGF 单字母坐标（0='a', 14='o'）
QChar SgfSerializer::toSgfChar(int idx)
{
    return QChar('a' + idx);
}

// 将 SGF 字母坐标解析为整数索引
int SgfSerializer::fromSgfChar(QChar c)
{
    return c.toLatin1() - 'a';
}

QString SgfSerializer::exportToSgf(const GameRecord &record, const QString &playerName)
{
    // SGF 根节点属性
    QString sgf;
    sgf += "(;";
    sgf += "FF[4]";
    sgf += "GM[4]";          // 五子棋
    sgf += "SZ[15]";
    sgf += "CA[UTF-8]";

    // 对局元数据
    if (!playerName.isEmpty()) {
        // 根据先手决定玩家颜色
        if (record.playerStarted) {
            sgf += QString("PB[%1]").arg(playerName);
            sgf += "PW[AI]";
        } else {
            sgf += "PB[AI]";
            sgf += QString("PW[%1]").arg(playerName);
        }
    }

    // 对局日期
    if (!record.finishedAt.isEmpty()) {
        sgf += QString("DT[%1]").arg(record.finishedAt.left(10));
    }

    // 胜负结果（R=Resign 表示认输/结束）
    if (record.playerWon) {
        sgf += record.playerStarted ? "RE[B+]" : "RE[W+]";
    } else {
        sgf += record.playerStarted ? "RE[W+]" : "RE[B+]";
    }

    // 落子序列
    for (const MoveRecord &mv : record.moves) {
        // SGF 坐标格式：[列行]，列先行后
        QString coord = QString("%1%2")
            .arg(toSgfChar(mv.col))
            .arg(toSgfChar(mv.row));
        if (mv.piece == BLACK) {
            sgf += QString(";B[%1]").arg(coord);
        } else if (mv.piece == WHITE) {
            sgf += QString(";W[%1]").arg(coord);
        }
    }

    sgf += ")";
    return sgf;
}

GameRecord SgfSerializer::importFromSgf(const QString &sgfText, bool &ok)
{
    ok = false;
    GameRecord record;

    // 基本格式检查
    QString text = sgfText.trimmed();
    if (!text.startsWith('(') || !text.endsWith(')')) {
        return record;
    }

    // 解析 SZ（棋盘尺寸）
    QRegularExpression szRe("SZ\\[(\\d+)\\]");
    auto szMatch = szRe.match(text);
    if (szMatch.hasMatch() && szMatch.captured(1).toInt() != 15) {
        // 非 15 路棋盘不支持
        return record;
    }

    // 解析胜负
    QRegularExpression reRe("RE\\[([^\\]]+)\\]");
    auto reMatch = reRe.match(text);
    if (reMatch.hasMatch()) {
        QString result = reMatch.captured(1);
        // 简单判断：B+ 表示黑棋赢
        record.playerWon = result.startsWith('B');
    }

    // 解析 PB/PW 判断先手
    QRegularExpression pbRe("PB\\[([^\\]]+)\\]");
    auto pbMatch = pbRe.match(text);
    if (pbMatch.hasMatch() && pbMatch.captured(1) != "AI") {
        record.playerStarted = true;  // 玩家执黑（先手）
    }

    // 解析日期
    QRegularExpression dtRe("DT\\[([^\\]]+)\\]");
    auto dtMatch = dtRe.match(text);
    if (dtMatch.hasMatch()) {
        record.finishedAt = dtMatch.captured(1);
    }

    // 解析落子序列
    QRegularExpression moveRe(";([BW])\\[([a-o]{2})\\]");
    QRegularExpressionMatchIterator it = moveRe.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        QString color = m.captured(1);
        QString coord = m.captured(2);
        if (coord.length() != 2) continue;

        int col = fromSgfChar(coord[0]);
        int row = fromSgfChar(coord[1]);

        // 边界校验
        if (row < 0 || row >= 15 || col < 0 || col >= 15) continue;

        MoveRecord mv;
        mv.row = row;
        mv.col = col;
        mv.piece = (color == "B") ? BLACK : WHITE;
        record.moves.append(mv);
    }

    record.moveCount = record.moves.size();
    ok = (record.moveCount > 0);
    return record;
}

bool SgfSerializer::saveToFile(const QString &filePath, const GameRecord &record, const QString &playerName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << exportToSgf(record, playerName);
    return true;
}

GameRecord SgfSerializer::loadFromFile(const QString &filePath, bool &ok)
{
    ok = false;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString text = in.readAll();
    return importFromSgf(text, ok);
}
