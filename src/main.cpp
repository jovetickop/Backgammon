#include "backgammon.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Backgammon w;
	w.show();
	return a.exec();
}
