#include "resultdialog.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ResultDialog::ResultDialog(QWidget *parent, bool playerWon, int moveCount, int totalGames, int playerWins, int aiWins)
	: QDialog(parent)
{
	setModal(true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowTitle(playerWon ? QString::fromUtf8(u8"\u672c\u5c40\u80dc\u5229") : QString::fromUtf8(u8"\u672c\u5c40\u7ed3\u675f"));
	setFixedSize(500, 340);
	setStyleSheet(
		"QDialog {"
		"background-color: rgb(236, 241, 247);"
		"}"
		"QFrame#card {"
		"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"stop:0 rgba(255,255,255,248), stop:1 rgba(229,236,244,248));"
		"border: 1px solid rgba(255,255,255,220);"
		"border-radius: 24px;"
		"}"
		"QLabel#badge {"
		"color: white;"
		"font-size: 24px;"
		"font-weight: 700;"
		"padding: 12px 18px;"
		"border-radius: 18px;"
		"background-color: rgba(69, 91, 125, 220);"
		"}"
		"QLabel#title {"
		"color: rgb(41, 52, 70);"
		"font-size: 28px;"
		"font-weight: 800;"
		"}"
		"QLabel#subtitle {"
		"color: rgb(97, 109, 126);"
		"font-size: 16px;"
		"}"
		"QLabel#infoCard {"
		"background-color: rgba(255,255,255,188);"
		"border: 1px solid rgba(221,228,236,220);"
		"border-radius: 16px;"
		"padding: 14px 16px;"
		"color: rgb(62, 74, 93);"
		"font-size: 18px;"
		"font-weight: 700;"
		"}"
		"QPushButton {"
		"min-height: 48px;"
		"padding: 10px 20px;"
		"border: none;"
		"border-radius: 14px;"
		"font-size: 18px;"
		"font-weight: 700;"
		"color: white;"
		"background-color: rgb(63, 84, 117);"
		"}"
		"QPushButton:hover {"
		"background-color: rgb(76, 99, 137);"
		"}"
		"QPushButton:pressed {"
		"background-color: rgb(53, 72, 100);"
		"}");

	QVBoxLayout *rootLayout = new QVBoxLayout(this);
	rootLayout->setContentsMargins(18, 18, 18, 18);

	QFrame *card = new QFrame(this);
	card->setObjectName("card");
	rootLayout->addWidget(card);

	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
	shadow->setBlurRadius(34);
	shadow->setOffset(0, 12);
	shadow->setColor(QColor(65, 82, 106, 80));
	card->setGraphicsEffect(shadow);

	QVBoxLayout *cardLayout = new QVBoxLayout(card);
	cardLayout->setContentsMargins(28, 28, 28, 24);
	cardLayout->setSpacing(18);

	QLabel *badge = new QLabel(playerWon ? QString::fromUtf8(u8"\u4f60\u8d62\u4e86") : QString::fromUtf8("AI \u83b7\u80dc"), card);
	badge->setObjectName("badge");
	badge->setAlignment(Qt::AlignCenter);
	if (playerWon)
	{
		badge->setStyleSheet("QLabel#badge { background-color: rgba(71, 119, 88, 220); color: white; font-size: 24px; font-weight: 700; padding: 12px 18px; border-radius: 18px; }");
	}
	cardLayout->addWidget(badge, 0, Qt::AlignLeft);

	QLabel *title = new QLabel(playerWon ? QString::fromUtf8(u8"\u8fd9\u4e00\u5c40\u4e0b\u5f97\u5f88\u7a33") : QString::fromUtf8(u8"\u8fd9\u5c40\u88ab AI \u62ff\u4e0b\u4e86"), card);
	title->setObjectName("title");
	cardLayout->addWidget(title);

	QLabel *subtitle = new QLabel(QString::fromUtf8(u8"\u672c\u5c40\u5171 %1 \u624b\uff0c\u5f53\u524d\u7d2f\u8ba1 %2 \u5c40").arg(moveCount).arg(totalGames), card);
	subtitle->setObjectName("subtitle");
	subtitle->setWordWrap(true);
	cardLayout->addWidget(subtitle);

	QHBoxLayout *infoLayout = new QHBoxLayout();
	infoLayout->setSpacing(12);
	cardLayout->addLayout(infoLayout);

	QLabel *playerCard = new QLabel(QString::fromUtf8(u8"\u6211\n%1 \u80dc").arg(playerWins), card);
	playerCard->setObjectName("infoCard");
	playerCard->setAlignment(Qt::AlignCenter);
	infoLayout->addWidget(playerCard);

	QLabel *aiCard = new QLabel(QString::fromUtf8("AI\n%1 \u80dc").arg(aiWins), card);
	aiCard->setObjectName("infoCard");
	aiCard->setAlignment(Qt::AlignCenter);
	infoLayout->addWidget(aiCard);

	cardLayout->addStretch(1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
	QPushButton *okButton = buttonBox->addButton(QString::fromUtf8(u8"\u7ee7\u7eed"), QDialogButtonBox::AcceptRole);
	QObject::connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
	cardLayout->addWidget(buttonBox);
}
