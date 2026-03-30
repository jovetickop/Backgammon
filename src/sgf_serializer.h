#pragma once

#include <QString>
#include "playerstatsstore.h"

// SGF（Smart Game Format）序列化器：支持五子棋对局的导出和导入。
// 使用 GM[4]（五子棋）标准，坐标以字母编码（a=0, b=1, ..., o=14）。
class SgfSerializer
{
public:
    // 将一局棋谱导出为 SGF 格式字符串。
    // record: 包含落子序列和对局元数据的 GameRecord。
    // playerName: 玩家名称，写入 PB/PW 标签。
    // 返回：SGF 格式字符串；出错时返回空字符串。
    static QString exportToSgf(const GameRecord &record, const QString &playerName);

    // 将 SGF 格式字符串解析为 GameRecord。
    // sgfText: SGF 格式字符串内容。
    // ok: 输出参数，解析成功时为 true，否则为 false。
    // 返回：解析出的 GameRecord；失败时返回空记录。
    static GameRecord importFromSgf(const QString &sgfText, bool &ok);

    // 将 SGF 文件写入磁盘。
    // filePath: 目标文件路径。
    // record: 待导出的对局记录。
    // playerName: 玩家名称。
    // 返回：写入成功返回 true。
    static bool saveToFile(const QString &filePath, const GameRecord &record, const QString &playerName);

    // 从磁盘读取 SGF 文件并解析。
    // filePath: SGF 文件路径。
    // ok: 输出参数，解析成功时为 true。
    // 返回：解析出的 GameRecord。
    static GameRecord loadFromFile(const QString &filePath, bool &ok);

private:
    // 将棋盘行列坐标转换为 SGF 字母坐标（0 -> 'a'）。
    static QChar toSgfChar(int idx);
    // 将 SGF 字母坐标解析回整数索引（'a' -> 0）。
    static int fromSgfChar(QChar c);
};
