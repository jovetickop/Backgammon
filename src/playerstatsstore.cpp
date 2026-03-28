#include "playerstatsstore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

PlayerStatsStore::PlayerStatsStore()
{
}

bool PlayerStatsStore::Load()
{
	m_records.clear();
	m_lastUser.clear();
	m_recentUsers.clear();

	QFile file(StoragePath());
	if (!file.exists())
		return true;

	if (!file.open(QIODevice::ReadOnly))
		return false;

	const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
	if (!document.isObject())
		return false;

	const QJsonObject root = document.object();
	QJsonObject playersObject;
	bool hasRecentUsersMetadata = false;

	if (root.contains("players") && root.value("players").isObject())
	{
		playersObject = root.value("players").toObject();
		m_lastUser = root.value("lastUser").toString().trimmed();
		hasRecentUsersMetadata = root.contains("recentUsers");

		const QJsonArray recentUsersArray = root.value("recentUsers").toArray();
		for (int i = 0; i < recentUsersArray.size(); ++i)
		{
			const QString userName = recentUsersArray[i].toString().trimmed();
			if (!userName.isEmpty() && !m_recentUsers.contains(userName, Qt::CaseInsensitive))
				m_recentUsers.push_back(userName);
		}
	}
	else
	{
		// Backward compatibility for the older flat players-only format.
		playersObject = root;
	}

	for (QJsonObject::const_iterator it = playersObject.begin(); it != playersObject.end(); ++it)
	{
		if (!it.value().isObject())
			continue;

		const QJsonObject item = it.value().toObject();
		PlayerRecord record;
		record.displayName = item.value("displayName").toString().trimmed();
		record.wins = item.value("wins").toInt();
		record.losses = item.value("losses").toInt();
		if (record.displayName.isEmpty())
			record.displayName = it.key();
		m_records.insert(it.key(), record);
	}

	if (!hasRecentUsersMetadata)
	{
		for (QHash<QString, PlayerRecord>::const_iterator it = m_records.begin(); it != m_records.end(); ++it)
		{
			const QString displayName = it.value().displayName.trimmed();
			if (!displayName.isEmpty() && !m_recentUsers.contains(displayName, Qt::CaseInsensitive))
				m_recentUsers.push_back(displayName);
		}
		m_recentUsers.sort(Qt::CaseInsensitive);
	}

	if (!m_lastUser.isEmpty())
		TouchRecentUser(m_lastUser);
	else if (!m_recentUsers.isEmpty())
		m_lastUser = m_recentUsers.first();

	return true;
}

bool PlayerStatsStore::Save() const
{
	const QString path = StoragePath();
	QDir dir;
	dir.mkpath(QFileInfo(path).absolutePath());

	QJsonObject playersObject;
	for (QHash<QString, PlayerRecord>::const_iterator it = m_records.begin(); it != m_records.end(); ++it)
	{
		QJsonObject item;
		item.insert("displayName", it.value().displayName);
		item.insert("wins", it.value().wins);
		item.insert("losses", it.value().losses);
		playersObject.insert(it.key(), item);
	}

	QJsonArray recentUsersArray;
	for (int i = 0; i < m_recentUsers.size(); ++i)
		recentUsersArray.push_back(m_recentUsers[i]);

	QJsonObject root;
	root.insert("players", playersObject);
	root.insert("lastUser", m_lastUser);
	root.insert("recentUsers", recentUsersArray);

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return false;

	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
	return true;
}

PlayerRecord PlayerStatsStore::RecordForUser(const QString &userName) const
{
	const QString trimmedName = userName.trimmed();
	const QString key = NormalizeUserName(trimmedName);

	if (m_records.contains(key))
	{
		PlayerRecord record = m_records.value(key);
		if (record.displayName.trimmed().isEmpty())
			record.displayName = trimmedName;
		return record;
	}

	PlayerRecord record;
	record.displayName = trimmedName;
	return record;
}

void PlayerStatsStore::SaveRecord(const PlayerRecord &record)
{
	const QString key = NormalizeUserName(record.displayName);
	if (key.isEmpty())
		return;

	PlayerRecord normalizedRecord = record;
	normalizedRecord.displayName = record.displayName.trimmed();
	m_records.insert(key, normalizedRecord);
}

QString PlayerStatsStore::LastUser() const
{
	return m_lastUser;
}

QStringList PlayerStatsStore::RecentUsers() const
{
	return m_recentUsers;
}

void PlayerStatsStore::SetLastUser(const QString &userName)
{
	m_lastUser = userName.trimmed();
}

void PlayerStatsStore::TouchRecentUser(const QString &userName)
{
	const QString trimmedName = userName.trimmed();
	if (trimmedName.isEmpty())
		return;

	for (int i = m_recentUsers.size() - 1; i >= 0; --i)
	{
		if (QString::compare(m_recentUsers[i], trimmedName, Qt::CaseInsensitive) == 0)
			m_recentUsers.removeAt(i);
	}

	m_recentUsers.push_front(trimmedName);
	while (m_recentUsers.size() > 8)
		m_recentUsers.removeLast();
}

QString PlayerStatsStore::StoragePath() const
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/players.json";
}

QString PlayerStatsStore::NormalizeUserName(const QString &userName) const
{
	return userName.trimmed().toLower();
}
