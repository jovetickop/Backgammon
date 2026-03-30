#include "backgammon.h"
#include "logindialog.h"
#include "playerstatsstore.h"

#include <QtWidgets/QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// 设置全局统一字体，使用抗锯齿渲染策略提高清晰度
	// 优先使用微软雅黑，其次尝试系统默认
	QFont defaultFont("Microsoft YaHei", 10);
	defaultFont.setStyleStrategy(QFont::PreferAntialias);
	a.setFont(defaultFont);

	PlayerStatsStore statsStore;
	statsStore.Load();

	LoginDialog loginDialog(&statsStore);
	if (loginDialog.exec() != QDialog::Accepted)
		return 0;

	const QString userName = loginDialog.UserName();
	statsStore.SetLastUser(userName);
	statsStore.TouchRecentUser(userName);
	statsStore.Save();

	const PlayerRecord playerRecord = statsStore.RecordForUser(userName);
	Backgammon w(&statsStore, playerRecord);
	w.show();
	return a.exec();
}
