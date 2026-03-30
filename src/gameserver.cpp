#include "gameserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>

GameServer::GameServer(QObject *parent)
    : QObject(parent)
    , m_pServer(new QTcpServer(this))
    , m_pClient(nullptr)
{
    connect(m_pServer, &QTcpServer::newConnection, this, &GameServer::onNewConnection);
}

GameServer::~GameServer()
{
    stop();
}

bool GameServer::startListening(quint16 port)
{
    if (!m_pServer->listen(QHostAddress::Any, port)) {
        emit errorOccurred(m_pServer->errorString());
        return false;
    }
    return true;
}

void GameServer::stop()
{
    if (m_pClient) {
        m_pClient->disconnectFromHost();
        m_pClient = nullptr;
    }
    m_pServer->close();
}

quint16 GameServer::listenPort() const
{
    return m_pServer->serverPort();
}

void GameServer::sendMove(int row, int col)
{
    if (!hasClient()) return;
    const QByteArray msg = QString("MOVE %1 %2\n").arg(row).arg(col).toUtf8();
    m_pClient->write(msg);
}

void GameServer::onNewConnection()
{
    // 只接受第一个客户端，其余忽略
    QTcpSocket *incoming = m_pServer->nextPendingConnection();
    if (m_pClient && m_pClient->isOpen()) {
        incoming->disconnectFromHost();
        return;
    }
    m_pClient = incoming;
    connect(m_pClient, &QTcpSocket::readyRead,    this, &GameServer::onClientReadyRead);
    connect(m_pClient, &QTcpSocket::disconnected, this, &GameServer::onClientDisconnected);
    emit clientConnected();
}

void GameServer::onClientReadyRead()
{
    m_readBuf += m_pClient->readAll();
    // 按行切分处理消息（防粘包）
    while (m_readBuf.contains('\n')) {
        const int idx = m_readBuf.indexOf('\n');
        const QByteArray line = m_readBuf.left(idx).trimmed();
        m_readBuf = m_readBuf.mid(idx + 1);
        if (line.startsWith("MOVE ")) {
            const QList<QByteArray> parts = line.split(' ');
            if (parts.size() == 3) {
                const int row = parts[1].toInt();
                const int col = parts[2].toInt();
                emit moveReceived(row, col);
            }
        }
    }
}

void GameServer::onClientDisconnected()
{
    m_pClient = nullptr;
    m_readBuf.clear();
    emit clientDisconnected();
}
