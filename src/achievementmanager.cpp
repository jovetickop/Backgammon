#include "achievementmanager.h"

#include <QDateTime>
#include <QSettings>

AchievementManager::AchievementManager(QObject *parent)
    : QObject(parent)
{
    // 定义全部成就（顺序固定，load() 依赖 id 匹配）
    m_achievements = {
        { "first_win",   QString::fromUtf8("初出茅庐"), QString::fromUtf8("赢得第一局对战"),           false, {} },
        { "win_streak3", QString::fromUtf8("连胜高手"), QString::fromUtf8("连续赢得 3 局对战"),        false, {} },
        { "blitz_win",   QString::fromUtf8("闪电战"),   QString::fromUtf8("在 20 步以内获胜"),          false, {} },
        { "iron_wall",   QString::fromUtf8("铁壁防守"), QString::fromUtf8("成功防守对手的连四威胁"),   false, {} },
        { "veteran",     QString::fromUtf8("百战老将"), QString::fromUtf8("累计完成 10 局对局"),        false, {} },
    };
    load();
}

void AchievementManager::load()
{
    QSettings settings;
    settings.beginGroup("achievements");
    for (auto &a : m_achievements) {
        a.unlocked   = settings.value(a.id + "/unlocked", false).toBool();
        a.unlockedAt = settings.value(a.id + "/unlockedAt", QString()).toString();
    }
    settings.endGroup();
}

void AchievementManager::save() const
{
    QSettings settings;
    settings.beginGroup("achievements");
    for (const auto &a : m_achievements) {
        settings.setValue(a.id + "/unlocked",   a.unlocked);
        settings.setValue(a.id + "/unlockedAt", a.unlockedAt);
    }
    settings.endGroup();
}

bool AchievementManager::unlock(const QString &id)
{
    for (auto &a : m_achievements) {
        if (a.id == id && !a.unlocked) {
            a.unlocked   = true;
            a.unlockedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            return true;
        }
    }
    return false;
}

void AchievementManager::checkAndUnlock(bool playerWon, int moveCount,
                                         int totalGames, int consecutiveWins,
                                         bool opponentHadOpenFour)
{
    QVector<Achievement> newlyUnlocked;

    auto tryUnlock = [&](const QString &id) {
        if (unlock(id)) {
            for (const auto &a : m_achievements)
                if (a.id == id) { newlyUnlocked.push_back(a); break; }
        }
    };

    if (playerWon)             tryUnlock("first_win");
    if (consecutiveWins >= 3)  tryUnlock("win_streak3");
    if (playerWon && moveCount <= 20) tryUnlock("blitz_win");
    if (opponentHadOpenFour)   tryUnlock("iron_wall");
    if (totalGames >= 10)      tryUnlock("veteran");

    if (!newlyUnlocked.isEmpty()) {
        save();
        emit achievementsUnlocked(newlyUnlocked);
    }
}
