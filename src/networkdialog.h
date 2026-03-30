#pragma once

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;
class QRadioButton;
class GameServer;
class GameClient;

// 局域网对战入口对话框：选择创建房间（主机）或加入房间（客户端）。
// 连接成功后 accept()，外部通过 isHost()/remoteAddress() 获取连接信息。
class NetworkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NetworkDialog(QWidget *parent = nullptr);
    ~NetworkDialog() override;

    // 当前是否为主机（服务端）角色。
    bool isHost() const { return m_isHost; }

    // 已建立的 GameServer（主机模式，调用方接管所有权）。
    GameServer *takeServer();

    // 已建立的 GameClient（客户端模式，调用方接管所有权）。
    GameClient *takeClient();

private slots:
    void onCreateRoom();
    void onJoinRoom();
    void onServerClientConnected();
    void onClientConnectedToServer();
    void onNetworkError(const QString &msg);

private:
    QRadioButton *m_pHostRadio;
    QRadioButton *m_pClientRadio;
    QLineEdit    *m_pHostEdit;    // 目标 IP（客户端模式）
    QLineEdit    *m_pPortEdit;    // 端口
    QLabel       *m_pStatusLabel;
    QPushButton  *m_pConnectBtn;

    bool          m_isHost = false;
    GameServer   *m_pServer = nullptr;
    GameClient   *m_pClient = nullptr;
};
