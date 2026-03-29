#include "backgammon.h"
#include "logindialog.h"
#include "playerstatsstore.h"

#include <QtWidgets/QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	// 设置全局统一字体
	QFont defaultFont("Source Han Sans CN", 10);
	defaultFont.setStyleStrategy(QFont::PreferOutline);
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
