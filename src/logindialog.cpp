#include "logindialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "playerstatsstore.h"

namespace
{
	int WinRate(int wins, int losses)
	{
		const int totalGames = wins + losses;
		return totalGames == 0 ? 0 : qRound(wins * 100.0 / totalGames);
	}
}

LoginDialog::LoginDialog(PlayerStatsStore *statsStore, QWidget *parent)
	: QDialog(parent)
	, m_pStatsStore(statsStore)
	, m_pUserNameCombo(0)
	, m_pHintLabel(0)
	, m_pGamesLabel(0)
	, m_pWinsLabel(0)
	, m_pLossesLabel(0)
	, m_pRateLabel(0)
	, m_pLoginButton(0)
{
	setModal(true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowTitle(QString::fromUtf8(u8"登录"));
	setFixedSize(560, 440);
	setStyleSheet(
		"QDialog {"
		"background: qradialgradient(cx:0.18, cy:0.12, radius:1.2, stop:0 rgba(255,255,255,236), stop:1 rgba(216,226,238,236));"
		"}"
		"QFrame#card {"
		"background-color: rgba(255,255,255,214);"
		"border: 1px solid rgba(255,255,255,220);"
		"border-radius: 26px;"
		"}"
		"QLabel#title {"
		"color: rgb(41, 52, 70);"
		"font-size: 30px;"
		"font-weight: 800;"
		"}"
		"QLabel#subtitle {"
		"color: rgb(94, 106, 124);"
		"font-size: 16px;"
		"}"
		"QComboBox {"
		"min-height: 54px;"
		"padding: 0 16px;"
		"font-size: 20px;"
		"font-weight: 600;"
		"border: 1px solid rgba(190,203,220,220);"
		"border-radius: 16px;"
		"background-color: rgba(255,255,255,240);"
		"color: rgb(47, 58, 76);"
		"}"
		"QComboBox::drop-down {"
		"width: 42px;"
		"border: none;"
		"}"
		"QComboBox QAbstractItemView {"
		"border: 1px solid rgba(190,203,220,220);"
		"selection-background-color: rgba(223,231,240,220);"
		"selection-color: rgb(47, 58, 76);"
		"font-size: 18px;"
		"}"
		"QLabel#hintLabel {"
		"color: rgb(99, 112, 128);"
		"font-size: 14px;"
		"}"
		"QLabel#sectionTitle {"
		"color: rgb(58, 70, 90);"
		"font-size: 20px;"
		"font-weight: 700;"
		"}"
		"QLabel#statCard {"
		"background-color: rgba(247,250,253,228);"
		"border: 1px solid rgba(219,228,238,225);"
		"border-radius: 16px;"
		"padding: 14px 16px;"
		"font-size: 17px;"
		"font-weight: 700;"
		"color: rgb(63, 75, 95);"
		"}"
		"QPushButton {"
		"min-height: 50px;"
		"padding: 10px 22px;"
		"border: none;"
		"border-radius: 14px;"
		"font-size: 18px;"
		"font-weight: 700;"
		"}"
		"QPushButton#loginButton {"
		"background-color: rgb(63, 84, 117);"
		"color: white;"
		"}"
		"QPushButton#loginButton:hover {"
		"background-color: rgb(76, 99, 137);"
		"}"
		"QPushButton#loginButton:pressed {"
		"background-color: rgb(53, 72, 100);"
		"}"
		"QPushButton#loginButton:disabled {"
		"background-color: rgb(168, 178, 194);"
		"}");

	QVBoxLayout *rootLayout = new QVBoxLayout(this);
	rootLayout->setContentsMargins(22, 22, 22, 22);

	QFrame *card = new QFrame(this);
	card->setObjectName("card");
	rootLayout->addWidget(card);

	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
	shadow->setBlurRadius(36);
	shadow->setOffset(0, 14);
	shadow->setColor(QColor(63, 82, 106, 82));
	card->setGraphicsEffect(shadow);

	QVBoxLayout *cardLayout = new QVBoxLayout(card);
	cardLayout->setContentsMargins(28, 28, 28, 24);
	cardLayout->setSpacing(16);

	QLabel *title = new QLabel(QString::fromUtf8(u8"选择你的用户名"), card);
	title->setObjectName("title");
	cardLayout->addWidget(title);

	QLabel *subtitle = new QLabel(
		QString::fromUtf8(u8"输入用户名后登录。输入框会默认显示上一次登录的用户，也支持从下拉框选择最近登录过的用户。"),
		card);
	subtitle->setObjectName("subtitle");
	subtitle->setWordWrap(true);
	cardLayout->addWidget(subtitle);

	m_pUserNameCombo = new QComboBox(card);
	m_pUserNameCombo->setEditable(true);
	m_pUserNameCombo->setInsertPolicy(QComboBox::NoInsert);
	m_pUserNameCombo->setMaxVisibleItems(8);
	m_pUserNameCombo->setPlaceholderText(QString::fromUtf8(u8"例如：Amber"));
	if (QLineEdit *lineEdit = m_pUserNameCombo->lineEdit())
	{
		lineEdit->setPlaceholderText(QString::fromUtf8(u8"例如：Amber"));
		lineEdit->setClearButtonEnabled(true);
	}
	if (m_pStatsStore)
		m_pUserNameCombo->addItems(m_pStatsStore->RecentUsers());
	cardLayout->addWidget(m_pUserNameCombo);

	const QString lastUser = m_pStatsStore ? m_pStatsStore->LastUser() : QString();
	if (!lastUser.isEmpty())
		m_pUserNameCombo->setCurrentText(lastUser);

	m_pHintLabel = new QLabel(card);
	m_pHintLabel->setObjectName("hintLabel");
	cardLayout->addWidget(m_pHintLabel);

	QLabel *sectionTitle = new QLabel(QString::fromUtf8(u8"历史数据预览"), card);
	sectionTitle->setObjectName("sectionTitle");
	cardLayout->addWidget(sectionTitle);

	QHBoxLayout *statsRowOne = new QHBoxLayout();
	statsRowOne->setSpacing(12);
	cardLayout->addLayout(statsRowOne);

	m_pGamesLabel = new QLabel(card);
	m_pGamesLabel->setObjectName("statCard");
	m_pGamesLabel->setAlignment(Qt::AlignCenter);
	statsRowOne->addWidget(m_pGamesLabel);

	m_pRateLabel = new QLabel(card);
	m_pRateLabel->setObjectName("statCard");
	m_pRateLabel->setAlignment(Qt::AlignCenter);
	statsRowOne->addWidget(m_pRateLabel);

	QHBoxLayout *statsRowTwo = new QHBoxLayout();
	statsRowTwo->setSpacing(12);
	cardLayout->addLayout(statsRowTwo);

	m_pWinsLabel = new QLabel(card);
	m_pWinsLabel->setObjectName("statCard");
	m_pWinsLabel->setAlignment(Qt::AlignCenter);
	statsRowTwo->addWidget(m_pWinsLabel);

	m_pLossesLabel = new QLabel(card);
	m_pLossesLabel->setObjectName("statCard");
	m_pLossesLabel->setAlignment(Qt::AlignCenter);
	statsRowTwo->addWidget(m_pLossesLabel);

	cardLayout->addStretch(1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
	m_pLoginButton = buttonBox->addButton(QString::fromUtf8(u8"进入对局"), QDialogButtonBox::AcceptRole);
	m_pLoginButton->setObjectName("loginButton");
	QPushButton *cancelButton = buttonBox->addButton(QString::fromUtf8(u8"退出"), QDialogButtonBox::RejectRole);
	QObject::connect(m_pLoginButton, &QPushButton::clicked, this, &LoginDialog::AcceptLogin);
	QObject::connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	cardLayout->addWidget(buttonBox);

	QObject::connect(m_pUserNameCombo, &QComboBox::currentTextChanged, this, &LoginDialog::UpdatePreview);
	if (m_pUserNameCombo->lineEdit())
		QObject::connect(m_pUserNameCombo->lineEdit(), &QLineEdit::textChanged, this, &LoginDialog::UpdatePreview);

	UpdatePreview();
}

QString LoginDialog::UserName() const
{
	return m_pUserNameCombo->currentText().trimmed();
}

void LoginDialog::UpdatePreview()
{
	const QString userName = UserName();
	const bool hasName = !userName.isEmpty();
	m_pLoginButton->setEnabled(hasName);

	PlayerRecord record;
	if (m_pStatsStore && hasName)
		record = m_pStatsStore->RecordForUser(userName);

	const int totalGames = record.wins + record.losses;
	m_pGamesLabel->setText(QString::fromUtf8(u8"累计局数\n%1").arg(totalGames));
	m_pWinsLabel->setText(QString::fromUtf8(u8"我的胜场\n%1").arg(record.wins));
	m_pLossesLabel->setText(QString::fromUtf8(u8"AI 胜场\n%1").arg(record.losses));
	m_pRateLabel->setText(QString::fromUtf8(u8"历史胜率\n%1%").arg(WinRate(record.wins, record.losses)));

	if (!hasName)
	{
		m_pHintLabel->setText(QString::fromUtf8(u8"输入用户名后即可查看该用户的历史对局数据。"));
	}
	else if (totalGames == 0)
	{
		m_pHintLabel->setText(QString::fromUtf8(u8"这是一个新用户，历史对局将从 0 开始记录。"));
	}
	else
	{
		m_pHintLabel->setText(QString::fromUtf8(u8"检测到已有历史记录，登录后会继续累计。"));
	}
}

void LoginDialog::AcceptLogin()
{
	if (UserName().isEmpty())
		return;

	accept();
}
