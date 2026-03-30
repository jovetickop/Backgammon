#ifndef PLAYERSTATSSTORE_H
#define PLAYERSTATSSTORE_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>

#include "types.h"

struct MoveRecord
{
	int row = 0;
	int col = 0;
	ePiece piece = NONE;
};

struct GameRecord
{
	QString finishedAt;
	bool playerWon = false;
	bool playerStarted = false;
	int moveCount = 0;
	QVector<MoveRecord> moves;
};

struct PlayerRecord
{
	QString displayName;
	int wins = 0;
	int losses = 0;
	QString preferredStarter = "ai";
	QVector<GameRecord> games;
};

class PlayerStatsStore
{
public:
	PlayerStatsStore();

	bool Load();
	bool Save() const;

	PlayerRecord RecordForUser(const QString &userName) const;
	void SaveRecord(const PlayerRecord &record);
	// 删除指定用户档案（不存在时无操作）
	void DeleteRecord(const QString &userName);
	// 所有已知用户名列表
	QStringList AllUsers() const;

	QString LastUser() const;
	QStringList RecentUsers() const;
	void SetLastUser(const QString &userName);
	void TouchRecentUser(const QString &userName);

	QString StoragePath() const;

private:
	QString NormalizeUserName(const QString &userName) const;

	QHash<QString, PlayerRecord> m_records;
	QString m_lastUser;
	QStringList m_recentUsers;
};

#endif // PLAYERSTATSSTORE_H
