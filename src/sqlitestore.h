#pragma once

#include "playerstatsstore.h"
#include <QString>

// SQLite 存储层：替代 JSON 文件，提供更高效的查询和索引。
// 数据库文件路径：AppData/Backgammon/backgammon.db
class SqliteStore
{
public:
    SqliteStore();
    ~SqliteStore();

    // 打开或创建数据库，建表
    bool Open();

    // 关闭数据库连接
    void Close();

    // 是否已打开
    bool IsOpen() const { return m_open; }

    // 从 JSON players.json 迁移数据（幂等，已迁移则跳过）
    bool MigrateFromJson(const QString& jsonPath);

    // 读取指定用户记录
    PlayerRecord RecordForUser(const QString& userName) const;

    // 写入用户记录（插入或更新）
    bool SaveRecord(const PlayerRecord& record);

    // 获取最近用户列表
    QStringList RecentUsers() const;

    // 获取/设置最后登录用户
    QString LastUser() const;
    bool SetLastUser(const QString& userName);

    // 将用户移到最近列表前端
    bool TouchRecentUser(const QString& userName);

    // 数据库文件路径
    static QString DbPath();

private:
    // 建表 SQL
    bool CreateTables();

    // 检查迁移标记
    bool IsMigrated() const;
    void SetMigrated();

    bool m_open = false;
    QString m_connectionName; // Qt SQL 连接名（唯一标识）
};
