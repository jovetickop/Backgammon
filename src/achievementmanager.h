#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>

// 成就数据结构
struct Achievement {
    QString id;           // 唯一标识
    QString name;         // 名称
    QString description;  // 描述
    bool unlocked;        // 是否已解锁
    QString unlockedAt;   // 解锁时间
};

// 成就管理器：检查解锁条件、持久化、通知
class AchievementManager : public QObject
{
    Q_OBJECT

public:
    explicit AchievementManager(QObject *parent = nullptr);

    // 对局结束后调用，检查并解锁成就
    // playerWon: 玩家是否获胜
    // moveCount: 本局步数
    // totalGames: 累计对局数
    // consecutiveWins: 当前连胜数
    // opponentHadOpenFour: 对手是否出现过连四被防守
    void checkAndUnlock(bool playerWon, int moveCount, int totalGames,
                        int consecutiveWins, bool opponentHadOpenFour);

    // 获取所有成就列表
    const QVector<Achievement>& achievements() const { return m_achievements; }

    // 从 QSettings 加载持久化数据
    void load();
    // 持久化到 QSettings
    void save() const;

signals:
    // 有新成就解锁时发出（可能同时解锁多个）
    void achievementsUnlocked(const QVector<Achievement> &newlyUnlocked);

private:
    // 内部解锁指定 id 的成就，若已解锁则跳过
    // 返回是否首次解锁
    bool unlock(const QString &id);

    QVector<Achievement> m_achievements;
};
