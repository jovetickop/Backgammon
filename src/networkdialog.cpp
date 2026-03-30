#include "networkdialog.h"
#include "gameserver.h"
#include "gameclient.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QNetworkInterface>
#include <QTimer>

NetworkDialog::NetworkDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8(u8"局域网对战"));
    setMinimumWidth(420);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);

    // 角色选择
    QGroupBox *roleBox = new QGroupBox(QString::fromUtf8(u8"选择角色"), this);
    QHBoxLayout *roleLayout = new QHBoxLayout(roleBox);
    m_pHostRadio   = new QRadioButton(QString::fromUtf8(u8"创建房间（主机）"), roleBox);
    m_pClientRadio = new QRadioButton(QString::fromUtf8(u8"加入房间（客户端）"), roleBox);
    m_pHostRadio->setChecked(true);
    roleLayout->addWidget(m_pHostRadio);
    roleLayout->addWidget(m_pClientRadio);
    mainLayout->addWidget(roleBox);

    // 连接参数
    QHBoxLayout *paramLayout = new QHBoxLayout;
    paramLayout->addWidget(new QLabel(QString::fromUtf8(u8"目标IP："), this));
    m_pHostEdit = new QLineEdit("127.0.0.1", this);
    m_pHostEdit->setEnabled(false); // 主机模式不需要填
    paramLayout->addWidget(m_pHostEdit);
    paramLayout->addWidget(new QLabel(QString::fromUtf8(u8"端口："), this));
    m_pPortEdit = new QLineEdit("12345", this);
    m_pPortEdit->setFixedWidth(70);
    paramLayout->addWidget(m_pPortEdit);
    mainLayout->addLayout(paramLayout);

    // 显示本机 IP 供对方输入
    const QString localIp = []() {
        for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
            if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
                !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
                for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                        return entry.ip().toString();
                }
            }
        }
        return QString("未知");
    }();
    QLabel *ipHint = new QLabel(
        QString::fromUtf8(u8"本机 IP：<b>%1</b>").arg(localIp), this);
    ipHint->setTextFormat(Qt::RichText);
    mainLayout->addWidget(ipHint);

    // 状态标签
    m_pStatusLabel = new QLabel(QString::fromUtf8(u8"等待操作..."), this);
    m_pStatusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_pStatusLabel);

    // 按钮
    m_pConnectBtn = new QPushButton(QString::fromUtf8(u8"创建房间"), this);
    QDialogButtonBox *btnBox = new QDialogButtonBox(this);
    btnBox->addButton(m_pConnectBtn, QDialogButtonBox::ActionRole);
    btnBox->addButton(QDialogButtonBox::Cancel);
    mainLayout->addWidget(btnBox);

    // 角色切换时更新 UI
    connect(m_pHostRadio, &QRadioButton::toggled, this, [this](bool host) {
        m_pHostEdit->setEnabled(!host);
        m_pConnectBtn->setText(host
            ? QString::fromUtf8(u8"创建房间")
            : QString::fromUtf8(u8"加入房间"));
    });

    connect(m_pConnectBtn, &QPushButton::clicked, this, [this]() {
        if (m_pHostRadio->isChecked()) onCreateRoom();
        else onJoinRoom();
    });
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

NetworkDialog::~NetworkDialog()
{
    // m_pServer/m_pClient 若已被 take 则由外部管理；否则随 QObject 树销毁
}

GameServer *NetworkDialog::takeServer()
{
    GameServer *s = m_pServer;
    if (s) s->setParent(nullptr);
    m_pServer = nullptr;
    return s;
}

GameClient *NetworkDialog::takeClient()
{
    GameClient *c = m_pClient;
    if (c) c->setParent(nullptr);
    m_pClient = nullptr;
    return c;
}

void NetworkDialog::onCreateRoom()
{
    const quint16 port = static_cast<quint16>(m_pPortEdit->text().toInt());
    m_pServer = new GameServer(this);
    connect(m_pServer, &GameServer::clientConnected,  this, &NetworkDialog::onServerClientConnected);
    connect(m_pServer, &GameServer::errorOccurred,    this, &NetworkDialog::onNetworkError);

    if (!m_pServer->startListening(port)) {
        delete m_pServer;
        m_pServer = nullptr;
        return;
    }
    m_pConnectBtn->setEnabled(false);
    m_pStatusLabel->setText(QString::fromUtf8(u8"房间已创建，等待对方连接（端口 %1）...").arg(port));
    m_isHost = true;
}

void NetworkDialog::onJoinRoom()
{
    const QString host = m_pHostEdit->text().trimmed();
    const quint16 port = static_cast<quint16>(m_pPortEdit->text().toInt());
    if (host.isEmpty()) {
        m_pStatusLabel->setText(QString::fromUtf8(u8"请输入主机 IP 地址。"));
        return;
    }
    m_pClient = new GameClient(this);
    connect(m_pClient, &GameClient::connected,     this, &NetworkDialog::onClientConnectedToServer);
    connect(m_pClient, &GameClient::errorOccurred, this, &NetworkDialog::onNetworkError);

    m_pConnectBtn->setEnabled(false);
    m_pStatusLabel->setText(QString::fromUtf8(u8"正在连接 %1:%2...").arg(host).arg(port));
    m_pClient->connectToServer(host, port);
    m_isHost = false;
}

void NetworkDialog::onServerClientConnected()
{
    m_pStatusLabel->setText(QString::fromUtf8(u8"对方已连接！即将开始对局..."));
    QTimer::singleShot(800, this, &QDialog::accept);
}

void NetworkDialog::onClientConnectedToServer()
{
    m_pStatusLabel->setText(QString::fromUtf8(u8"连接成功！即将开始对局..."));
    QTimer::singleShot(800, this, &QDialog::accept);
}

void NetworkDialog::onNetworkError(const QString &msg)
{
    m_pConnectBtn->setEnabled(true);
    m_pStatusLabel->setText(QString::fromUtf8(u8"错误：") + msg);
}
