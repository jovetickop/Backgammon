#include "playerstatsstore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace
{
	QString NormalizeStarter(const QString &starter)
	{
		return starter.trimmed().toLower() == "player" ? "player" : "ai";
	}

	void ComputeWinsAndLosses(PlayerRecord &record)
	{
		int wins = 0;
		int losses = 0;
		for (int i = 0; i < record.games.size(); ++i)
		{
			if (record.games[i].playerWon)
				++wins;
			else
				++losses;
		}
		record.wins = wins;
		record.losses = losses;
	}
}

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
		record.preferredStarter = NormalizeStarter(item.value("preferredStarter").toString());

		const QJsonArray gamesArray = item.value("games").toArray();
		for (int gameIndex = 0; gameIndex < gamesArray.size(); ++gameIndex)
		{
			if (!gamesArray[gameIndex].isObject())
				continue;

			const QJsonObject gameObject = gamesArray[gameIndex].toObject();
			GameRecord game;
			game.finishedAt = gameObject.value("finishedAt").toString().trimmed();
			game.playerWon = gameObject.value("playerWon").toBool();
			game.playerStarted = gameObject.value("playerStarted").toBool();
			game.moveCount = gameObject.value("moveCount").toInt();

			const QJsonArray movesArray = gameObject.value("moves").toArray();
			for (int moveIndex = 0; moveIndex < movesArray.size(); ++moveIndex)
			{
				if (!movesArray[moveIndex].isObject())
					continue;

				const QJsonObject moveObject = movesArray[moveIndex].toObject();
				MoveRecord move;
				move.row = moveObject.value("row").toInt();
				move.col = moveObject.value("col").toInt();
				move.piece = static_cast<ePiece>(moveObject.value("piece").toInt());
				game.moves.push_back(move);
			}

			if (game.moveCount <= 0)
				game.moveCount = game.moves.size();

			record.games.push_back(game);
		}

		if (!record.games.isEmpty())
			ComputeWinsAndLosses(record);

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
		item.insert("preferredStarter", NormalizeStarter(it.value().preferredStarter));

		QJsonArray gamesArray;
		for (int gameIndex = 0; gameIndex < it.value().games.size(); ++gameIndex)
		{
			const GameRecord &game = it.value().games[gameIndex];
			QJsonObject gameObject;
			gameObject.insert("finishedAt", game.finishedAt);
			gameObject.insert("playerWon", game.playerWon);
			gameObject.insert("playerStarted", game.playerStarted);
			gameObject.insert("moveCount", game.moveCount);

			QJsonArray movesArray;
			for (int moveIndex = 0; moveIndex < game.moves.size(); ++moveIndex)
			{
				const MoveRecord &move = game.moves[moveIndex];
				QJsonObject moveObject;
				moveObject.insert("row", move.row);
				moveObject.insert("col", move.col);
				moveObject.insert("piece", static_cast<int>(move.piece));
				movesArray.push_back(moveObject);
			}

			gameObject.insert("moves", movesArray);
			gamesArray.push_back(gameObject);
		}

		item.insert("games", gamesArray);
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
		record.preferredStarter = NormalizeStarter(record.preferredStarter);
		return record;
	}

	PlayerRecord record;
	record.displayName = trimmedName;
	record.preferredStarter = "ai";
	return record;
}

void PlayerStatsStore::SaveRecord(const PlayerRecord &record)
{
	const QString key = NormalizeUserName(record.displayName);
	if (key.isEmpty())
		return;

	PlayerRecord normalizedRecord = record;
	normalizedRecord.displayName = record.displayName.trimmed();
	normalizedRecord.preferredStarter = NormalizeStarter(record.preferredStarter);
	if (!normalizedRecord.games.isEmpty())
		ComputeWinsAndLosses(normalizedRecord);
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
