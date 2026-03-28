#include "backgammon.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	// Qt 应用入口：初始化事件循环并展示主窗口。
	QApplication a(argc, argv);
	Backgammon w;
	w.show();
	return a.exec();
}
