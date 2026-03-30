#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

// 局域网对战服务端：监听端口，接受一个客户端连接，转发落子消息。
// 协议：每条消息为 "MOVE row col\n" 格式的 UTF-8 文本行。
class GameServer : public QObject
{
    Q_OBJECT
public:
    explicit GameServer(QObject *parent = nullptr);
    ~GameServer() override;

    // 开始监听指定端口。成功返回 true。
    bool startListening(quint16 port = 12345);

    // 停止监听并断开所有连接。
    void stop();

    // 向客户端发送落子消息。
    void sendMove(int row, int col);

    // 是否已有客户端连接。
    bool hasClient() const { return m_pClient != nullptr && m_pClient->isOpen(); }

    // 服务端监听的端口号。
    quint16 listenPort() const;

signals:
    // 客户端连接成功。
    void clientConnected();
    // 客户端断开连接。
    void clientDisconnected();
    // 收到客户端落子消息。
    void moveReceived(int row, int col);
    // 发生错误。
    void errorOccurred(const QString &msg);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

private:
    QTcpServer  *m_pServer;
    QTcpSocket  *m_pClient;  // 只支持单个客户端
    QByteArray   m_readBuf;  // 读缓冲（处理粘包）
};
