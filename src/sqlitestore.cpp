#include "sqlitestore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

static constexpr int RECENT_USERS_MAX = 8;

SqliteStore::SqliteStore()
    : m_connectionName(QStringLiteral("backgammon_db"))
{
}

SqliteStore::~SqliteStore()
{
    Close();
}

QString SqliteStore::DbPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + QStringLiteral("/backgammon.db");
}

bool SqliteStore::Open()
{
    const QString path = DbPath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    db.setDatabaseName(path);
    if (!db.open()) {
        return false;
    }
    m_open = true;
    return CreateTables();
}

void SqliteStore::Close()
{
    if (m_open) {
        QSqlDatabase::database(m_connectionName).close();
        QSqlDatabase::removeDatabase(m_connectionName);
        m_open = false;
    }
}

bool SqliteStore::CreateTables()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery q(db);

    // 玩家表
    if (!q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS players ("
        "  username TEXT PRIMARY KEY,"
        "  display_name TEXT NOT NULL DEFAULT '',"
        "  wins INTEGER NOT NULL DEFAULT 0,"
        "  losses INTEGER NOT NULL DEFAULT 0,"
        "  preferred_starter TEXT NOT NULL DEFAULT 'ai'"
        ")"))) return false;

    // 对局记录表（每局一行，moves 以 JSON 格式存储）
    if (!q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS game_records ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL,"
        "  finished_at TEXT NOT NULL DEFAULT '',"
        "  player_won INTEGER NOT NULL DEFAULT 0,"
        "  player_started INTEGER NOT NULL DEFAULT 0,"
        "  move_count INTEGER NOT NULL DEFAULT 0,"
        "  moves_json TEXT NOT NULL DEFAULT '[]'"
        ")"))) return false;

    // 索引：按用户名快速查询对局
    q.exec(QStringLiteral(
        "CREATE INDEX IF NOT EXISTS idx_game_records_username "
        "ON game_records(username)"));

    // 元数据表：存储 last_user、recent_users、migration_done 等
    if (!q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS metadata ("
        "  key TEXT PRIMARY KEY,"
        "  value TEXT NOT NULL DEFAULT ''"
        ")"))) return false;

    return true;
}

bool SqliteStore::IsMigrated() const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT value FROM metadata WHERE key = 'migrated'"));
    if (q.exec() && q.next())
        return q.value(0).toString() == QLatin1String("1");
    return false;
}

void SqliteStore::SetMigrated()
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO metadata(key, value) VALUES('migrated', '1')"));
    q.exec();
}

bool SqliteStore::MigrateFromJson(const QString& jsonPath)
{
    if (IsMigrated()) return true; // 已迁移，幂等

    QFile file(jsonPath);
    if (!file.exists()) {
        SetMigrated(); // 无 JSON 文件，标记为迁移完成
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) return false;

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) { SetMigrated(); return true; }

    const QJsonObject root = doc.object();
    QJsonObject playersObj;
    if (root.contains(QStringLiteral("players")) && root.value(QStringLiteral("players")).isObject())
        playersObj = root.value(QStringLiteral("players")).toObject();
    else
        playersObj = root;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    for (auto it = playersObj.begin(); it != playersObj.end(); ++it) {
        if (!it.value().isObject()) continue;
        const QJsonObject item = it.value().toObject();

        const QString username = it.key().trimmed().toLower();
        const QString displayName = item.value(QStringLiteral("displayName")).toString().trimmed();
        const int wins = item.value(QStringLiteral("wins")).toInt();
        const int losses = item.value(QStringLiteral("losses")).toInt();
        const QString starter = item.value(QStringLiteral("preferredStarter")).toString();

        QSqlQuery q(db);
        q.prepare(QStringLiteral(
            "INSERT OR IGNORE INTO players(username, display_name, wins, losses, preferred_starter)"
            " VALUES(?, ?, ?, ?, ?)"));
        q.addBindValue(username);
        q.addBindValue(displayName.isEmpty() ? it.key() : displayName);
        q.addBindValue(wins);
        q.addBindValue(losses);
        q.addBindValue(starter.isEmpty() ? QStringLiteral("ai") : starter);
        q.exec();

        const QJsonArray gamesArr = item.value(QStringLiteral("games")).toArray();
        for (int gi = 0; gi < gamesArr.size(); ++gi) {
            if (!gamesArr[gi].isObject()) continue;
            const QJsonObject game = gamesArr[gi].toObject();
            const QString finishedAt = game.value(QStringLiteral("finishedAt")).toString();
            const bool playerWon = game.value(QStringLiteral("playerWon")).toBool();
            const bool playerStarted = game.value(QStringLiteral("playerStarted")).toBool();
            const int moveCount = game.value(QStringLiteral("moveCount")).toInt();
            // moves 直接存为 JSON 字符串
            QJsonDocument movesDoc(game.value(QStringLiteral("moves")).toArray());
            const QString movesJson = QString::fromUtf8(movesDoc.toJson(QJsonDocument::Compact));

            QSqlQuery gq(db);
            gq.prepare(QStringLiteral(
                "INSERT INTO game_records"
                "(username, finished_at, player_won, player_started, move_count, moves_json)"
                " VALUES(?, ?, ?, ?, ?, ?)"));
            gq.addBindValue(username);
            gq.addBindValue(finishedAt);
            gq.addBindValue(playerWon ? 1 : 0);
            gq.addBindValue(playerStarted ? 1 : 0);
            gq.addBindValue(moveCount);
            gq.addBindValue(movesJson);
            gq.exec();
        }
    }

    // 迁移 lastUser / recentUsers
    const QString lastUser = root.value(QStringLiteral("lastUser")).toString().trimmed();
    if (!lastUser.isEmpty()) {
        QSqlQuery mq(db);
        mq.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO metadata(key, value) VALUES('last_user', ?)"));
        mq.addBindValue(lastUser);
        mq.exec();
    }
    const QJsonArray recentArr = root.value(QStringLiteral("recentUsers")).toArray();
    if (!recentArr.isEmpty()) {
        QJsonDocument rd(recentArr);
        QSqlQuery mq(db);
        mq.prepare(QStringLiteral(
            "INSERT OR REPLACE INTO metadata(key, value) VALUES('recent_users', ?)"));
        mq.addBindValue(QString::fromUtf8(rd.toJson(QJsonDocument::Compact)));
        mq.exec();
    }

    db.commit();
    SetMigrated();
    return true;
}

PlayerRecord SqliteStore::RecordForUser(const QString& userName) const
{
    const QString key = userName.trimmed().toLower();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    PlayerRecord record;
    record.displayName = userName.trimmed();
    record.preferredStarter = QStringLiteral("ai");

    // 读玩家基本信息
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "SELECT display_name, wins, losses, preferred_starter FROM players WHERE username = ?"));
    q.addBindValue(key);
    if (q.exec() && q.next()) {
        record.displayName = q.value(0).toString();
        record.wins = q.value(1).toInt();
        record.losses = q.value(2).toInt();
        record.preferredStarter = q.value(3).toString();
    }

    // 读对局记录（按时间倒序）
    QSqlQuery gq(db);
    gq.prepare(QStringLiteral(
        "SELECT finished_at, player_won, player_started, move_count, moves_json"
        " FROM game_records WHERE username = ? ORDER BY id DESC"));
    gq.addBindValue(key);
    if (gq.exec()) {
        while (gq.next()) {
            GameRecord game;
            game.finishedAt = gq.value(0).toString();
            game.playerWon = gq.value(1).toInt() != 0;
            game.playerStarted = gq.value(2).toInt() != 0;
            game.moveCount = gq.value(3).toInt();

            const QByteArray movesJson = gq.value(4).toString().toUtf8();
            const QJsonArray movesArr = QJsonDocument::fromJson(movesJson).array();
            for (int i = 0; i < movesArr.size(); ++i) {
                if (!movesArr[i].isObject()) continue;
                const QJsonObject mo = movesArr[i].toObject();
                MoveRecord mv;
                mv.row = mo.value(QStringLiteral("row")).toInt();
                mv.col = mo.value(QStringLiteral("col")).toInt();
                mv.piece = static_cast<ePiece>(mo.value(QStringLiteral("piece")).toInt());
                game.moves.push_back(mv);
            }
            if (game.moveCount <= 0) game.moveCount = game.moves.size();
            record.games.push_back(game);
        }
    }
    return record;
}

bool SqliteStore::SaveRecord(const PlayerRecord& record)
{
    const QString key = record.displayName.trimmed().toLower();
    if (key.isEmpty()) return false;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction();

    // 统计胜负
    int wins = 0, losses = 0;
    for (int i = 0; i < record.games.size(); ++i) {
        if (record.games[i].playerWon) ++wins; else ++losses;
    }

    // 插入或更新玩家表
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO players(username, display_name, wins, losses, preferred_starter)"
        " VALUES(?, ?, ?, ?, ?)"));
    q.addBindValue(key);
    q.addBindValue(record.displayName.trimmed());
    q.addBindValue(wins);
    q.addBindValue(losses);
    const QString starter = record.preferredStarter.trimmed().toLower() == QLatin1String("player")
                            ? QStringLiteral("player") : QStringLiteral("ai");
    q.addBindValue(starter);
    if (!q.exec()) { db.rollback(); return false; }

    // 删旧对局，重新插入（全量替换策略，数据量小时足够）
    QSqlQuery dq(db);
    dq.prepare(QStringLiteral("DELETE FROM game_records WHERE username = ?"));
    dq.addBindValue(key);
    dq.exec();

    for (int i = 0; i < record.games.size(); ++i) {
        const GameRecord& game = record.games[i];
        QJsonArray movesArr;
        for (int j = 0; j < game.moves.size(); ++j) {
            QJsonObject mo;
            mo[QStringLiteral("row")] = game.moves[j].row;
            mo[QStringLiteral("col")] = game.moves[j].col;
            mo[QStringLiteral("piece")] = static_cast<int>(game.moves[j].piece);
            movesArr.append(mo);
        }
        const QString movesJson = QString::fromUtf8(
            QJsonDocument(movesArr).toJson(QJsonDocument::Compact));

        QSqlQuery gq(db);
        gq.prepare(QStringLiteral(
            "INSERT INTO game_records"
            "(username, finished_at, player_won, player_started, move_count, moves_json)"
            " VALUES(?, ?, ?, ?, ?, ?)"));
        gq.addBindValue(key);
        gq.addBindValue(game.finishedAt);
        gq.addBindValue(game.playerWon ? 1 : 0);
        gq.addBindValue(game.playerStarted ? 1 : 0);
        gq.addBindValue(game.moveCount);
        gq.addBindValue(movesJson);
        gq.exec();
    }

    db.commit();
    return true;
}

QStringList SqliteStore::RecentUsers() const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT value FROM metadata WHERE key = 'recent_users'"));
    if (q.exec() && q.next()) {
        const QByteArray raw = q.value(0).toString().toUtf8();
        const QJsonArray arr = QJsonDocument::fromJson(raw).array();
        QStringList result;
        for (int i = 0; i < arr.size(); ++i)
            result << arr[i].toString();
        return result;
    }
    return {};
}

QString SqliteStore::LastUser() const
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral("SELECT value FROM metadata WHERE key = 'last_user'"));
    if (q.exec() && q.next())
        return q.value(0).toString();
    return {};
}

bool SqliteStore::SetLastUser(const QString& userName)
{
    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO metadata(key, value) VALUES('last_user', ?)"));
    q.addBindValue(userName.trimmed());
    return q.exec();
}

bool SqliteStore::TouchRecentUser(const QString& userName)
{
    const QString name = userName.trimmed();
    if (name.isEmpty()) return false;

    QStringList list = RecentUsers();
    // 移除旧的（大小写不敏感）
    for (int i = list.size() - 1; i >= 0; --i)
        if (list[i].compare(name, Qt::CaseInsensitive) == 0)
            list.removeAt(i);
    list.prepend(name);
    while (list.size() > RECENT_USERS_MAX) list.removeLast();

    QJsonArray arr;
    for (const QString& s : list) arr.append(s);
    const QString json = QString::fromUtf8(
        QJsonDocument(arr).toJson(QJsonDocument::Compact));

    QSqlQuery q(QSqlDatabase::database(m_connectionName));
    q.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO metadata(key, value) VALUES('recent_users', ?)"));
    q.addBindValue(json);
    return q.exec();
}
