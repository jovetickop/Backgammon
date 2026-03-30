#pragma once

#include <QDialog>
#include "achievementmanager.h"

// 成就列表对话框：显示已解锁/未解锁成就
class AchievementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AchievementDialog(const AchievementManager *manager, QWidget *parent = nullptr);
};
