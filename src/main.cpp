#include "gamebridge.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFont>
#include <QQuickStyle>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("五子棋 AI");
    a.setApplicationVersion("1.0");

    // 设置全局统一字体
    QFont defaultFont("Microsoft YaHei", 10);
    defaultFont.setStyleStrategy(QFont::PreferAntialias);
    a.setFont(defaultFont);

    // 使用 Qt Quick Controls 2 样式
    QQuickStyle::setStyle("Basic");

    // 创建 QML 应用引擎
    QQmlApplicationEngine engine;

    // 注册 GameBridge 到 QML
    GameBridge gameBridge;
    engine.rootContext()->setContextProperty("gameBridge", &gameBridge);

    // 加载 QML 界面（从文件系统加载）
    engine.load(QUrl::fromLocalFile("qml/GameUI.qml"));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return a.exec();
}
