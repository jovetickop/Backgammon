#include "gameclient.h"

GameClient::GameClient(QObject *parent)
    : QObject(parent)
    , m_pSocket(new QTcpSocket(this))
{
    connect(m_pSocket, &QTcpSocket::connected,    this, &GameClient::onConnected);
    connect(m_pSocket, &QTcpSocket::disconnected, this, &GameClient::onDisconnected);
    connect(m_pSocket, &QTcpSocket::readyRead,    this, &GameClient::onReadyRead);
    connect(m_pSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            this, &GameClient::onError);
}

GameClient::~GameClient()
{
    disconnectFromServer();
}

void GameClient::connectToServer(const QString &host, quint16 port)
{
    m_pSocket->connectToHost(host, port);
}

void GameClient::disconnectFromServer()
{
    if (m_pSocket->isOpen())
        m_pSocket->disconnectFromHost();
}

bool GameClient::isConnected() const
{
    return m_pSocket->state() == QAbstractSocket::ConnectedState;
}

void GameClient::sendMove(int row, int col)
{
    if (!isConnected()) return;
    const QByteArray msg = QString("MOVE %1 %2\n").arg(row).arg(col).toUtf8();
    m_pSocket->write(msg);
}

void GameClient::onConnected()
{
    emit connected();
}

void GameClient::onDisconnected()
{
    m_readBuf.clear();
    emit disconnected();
}

void GameClient::onReadyRead()
{
    m_readBuf += m_pSocket->readAll();
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

void GameClient::onError(QAbstractSocket::SocketError /*err*/)
{
    emit errorOccurred(m_pSocket->errorString());
}
