#include "backgammon.h"
#include "logindialog.h"
#include "playerstatsstore.h"

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

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
