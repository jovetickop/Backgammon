#pragma once

#include <QObject>
#include <QTcpSocket>

// 局域网对战客户端：连接服务端，收发落子消息。
// 协议：每条消息为 "MOVE row col\n" 格式的 UTF-8 文本行。
class GameClient : public QObject
{
    Q_OBJECT
public:
    explicit GameClient(QObject *parent = nullptr);
    ~GameClient() override;

    // 连接到指定主机和端口。
    void connectToServer(const QString &host, quint16 port = 12345);

    // 断开连接。
    void disconnectFromServer();

    // 向服务端发送落子消息。
    void sendMove(int row, int col);

    // 是否已连接。
    bool isConnected() const;

signals:
    // 连接成功。
    void connected();
    // 连接断开。
    void disconnected();
    // 收到服务端落子消息。
    void moveReceived(int row, int col);
    // 发生错误。
    void errorOccurred(const QString &msg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError err);

private:
    QTcpSocket *m_pSocket;
    QByteArray  m_readBuf;
};
