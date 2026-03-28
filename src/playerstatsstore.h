#ifndef PLAYERSTATSSTORE_H
#define PLAYERSTATSSTORE_H

#include <QHash>
#include <QString>
#include <QStringList>

struct PlayerRecord
{
	QString displayName;
	int wins = 0;
	int losses = 0;
};

class PlayerStatsStore
{
public:
	PlayerStatsStore();

	bool Load();
	bool Save() const;

	PlayerRecord RecordForUser(const QString &userName) const;
	void SaveRecord(const PlayerRecord &record);

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
