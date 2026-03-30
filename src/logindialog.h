#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QComboBox;
class QLabel;
class QPushButton;
class PlayerStatsStore;

class LoginDialog : public QDialog
{
	Q_OBJECT

public:
	explicit LoginDialog(PlayerStatsStore *statsStore, QWidget *parent = 0);

	QString UserName() const;

private slots:
	void UpdatePreview();
	void AcceptLogin();

private:
	PlayerStatsStore *m_pStatsStore;
	QComboBox *m_pUserNameCombo;
	QLabel *m_pHintLabel;
	QLabel *m_pGamesLabel;
	QLabel *m_pWinsLabel;
	QLabel *m_pLossesLabel;
	QLabel *m_pRateLabel;
	QPushButton *m_pLoginButton;
};

#endif // LOGINDIALOG_H
