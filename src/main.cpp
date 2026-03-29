#include "gamebridge.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFont>
#include <QQuickStyle>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("五子棋 AI");
    a.setApplicationVersion("1.0");

    // 调试输出
    qDebug() << "App dir:" << QCoreApplication::applicationDirPath();
    qDebug() << "CWD:" << QDir::currentPath();

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

    // 加载 QML 界面
    QString qmlPath = QCoreApplication::applicationDirPath() + "/qml/GameUI.qml";
    qDebug() << "Loading QML from:" << qmlPath;

    QUrl qmlUrl = QUrl::fromLocalFile(qmlPath);
    engine.load(qmlUrl);

    if (engine.rootObjects().isEmpty()) {
        qDebug() << "ERROR: No root objects loaded!";
        return -1;
    }

    qDebug() << "QML loaded successfully!";

    return a.exec();
}
