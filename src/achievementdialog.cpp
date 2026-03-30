#include "achievementdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

AchievementDialog::AchievementDialog(const AchievementManager *manager, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("\u6211\u7684\u6210\u5C31"));
    setMinimumSize(360, 420);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 12);

    QLabel *title = new QLabel(QString::fromUtf8("\u6210\u5C31\u5217\u8868"), this);
    QFont tf = title->font();
    tf.setPixelSize(20);
    tf.setBold(true);
    title->setFont(tf);
    root->addWidget(title);

    // 可滚动区域
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    QWidget *container = new QWidget(scroll);
    QVBoxLayout *list = new QVBoxLayout(container);
    list->setSpacing(8);
    list->setContentsMargins(0, 4, 0, 4);

    for (const Achievement &a : manager->achievements()) {
        QLabel *item = new QLabel(container);
        const QString icon = a.unlocked
            ? QString::fromUtf8("\u2705")   // ✅
            : QString::fromUtf8("\u274C");  // ❌
        const QString text = a.unlocked
            ? QString::fromUtf8("%1 <b>%2</b><br><small>%3<br>\u89E3\u9501\u4E8E %4</small>")
                  .arg(icon, a.name, a.description, a.unlockedAt)
            : QString::fromUtf8("%1 <b>%2</b><br><small>%3</small>")
                  .arg(icon, a.name, a.description);
        item->setText(text);
        item->setWordWrap(true);
        item->setStyleSheet(
            a.unlocked
            ? "background:#e8f5e9;border-radius:8px;padding:10px;"
            : "background:#f5f5f5;border-radius:8px;padding:10px;color:#999;"
        );
        list->addWidget(item);
    }
    list->addStretch();
    container->setLayout(list);
    scroll->setWidget(container);
    root->addWidget(scroll);

    QDialogButtonBox *btn = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(btn, &QDialogButtonBox::accepted, this, &QDialog::accept);
    root->addWidget(btn);
}
