#include "historydialog.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
	QString StarterText(bool playerStarted)
	{
		return playerStarted ? QString::fromUtf8(u8"\u6211\u5148\u624B") : QString::fromUtf8(u8"AI \u5148\u624B");
	}

	QString ResultText(bool playerWon)
	{
		return playerWon ? QString::fromUtf8(u8"\u80DC\u5229") : QString::fromUtf8(u8"\u5931\u5229");
	}

	QString MoveText(const MoveRecord &move)
	{
		return QString::fromUtf8(u8"(%1,%2)").arg(move.row + 1).arg(move.col + 1);
	}

	QString MovesPreview(const QVector<MoveRecord> &moves)
	{
		if (moves.isEmpty())
			return QString::fromUtf8(u8"\u6682\u65E0\u843D\u5B50\u5E8F\u5217");

		QStringList parts;
		const int previewCount = qMin(8, moves.size());
		for (int i = 0; i < previewCount; ++i)
			parts << MoveText(moves[i]);
		if (moves.size() > previewCount)
			parts << QString::fromUtf8(u8"...");
		return parts.join(", ");
	}
}

HistoryDialog::HistoryDialog(const QString &userName, const PlayerRecord &record, QWidget *parent)
	: QDialog(parent)
{
	setModal(true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowTitle(QString::fromUtf8(u8"\u5386\u53F2\u5BF9\u5C40"));
	setMinimumSize(760, 620);
	setStyleSheet(
		"* { font-family: \"Source Han Sans CN\", \"Noto Sans CJK SC\", \"Microsoft YaHei\", sans-serif; }"
		"QDialog {"
		"background-color: rgb(236, 241, 247);"
		"}"
		"QFrame#card {"
		"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
		"stop:0 rgba(255,255,255,248), stop:1 rgba(229,236,244,248));"
		"border: 1px solid rgba(255,255,255,220);"
		"border-radius: 24px;"
		"}"
		"QLabel#title {"
		"color: rgb(41, 52, 70);"
		"font-size: 28px;"
		"font-weight: 800;"
		"}"
		"QLabel#subtitle {"
		"color: rgb(96, 108, 124);"
		"font-size: 16px;"
		"}"
		"QListWidget {"
		"background-color: rgba(255,255,255,190);"
		"border: 1px solid rgba(221,228,236,220);"
		"border-radius: 18px;"
		"padding: 8px;"
		"font-size: 16px;"
		"color: rgb(56, 69, 89);"
		"}"
		"QListWidget::item {"
		"padding: 12px 14px;"
		"margin: 4px 0;"
		"border-radius: 12px;"
		"background-color: rgba(247,250,253,228);"
		"}"
		"QPushButton {"
		"min-height: 46px;"
		"padding: 10px 22px;"
		"border: none;"
		"border-radius: 14px;"
		"font-size: 18px;"
		"font-weight: 700;"
		"color: white;"
		"background-color: rgb(63, 84, 117);"
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
	cardLayout->setSpacing(16);

	QLabel *title = new QLabel(QString::fromUtf8(u8"%1 \u7684\u5386\u53F2\u5BF9\u5C40").arg(userName), card);
	title->setObjectName("title");
	cardLayout->addWidget(title);

	QLabel *subtitle = new QLabel(
		QString::fromUtf8(u8"\u7D2F\u8BA1 %1 \u5C40 | \u6211 %2 \u80DC | AI %3 \u80DC | \u9ED8\u8BA4\u5F00\u5C40\uFF1A%4")
			.arg(record.games.size())
			.arg(record.wins)
			.arg(record.losses)
			.arg(StarterText(record.preferredStarter == "player")),
		card);
	subtitle->setObjectName("subtitle");
	subtitle->setWordWrap(true);
	cardLayout->addWidget(subtitle);

	QListWidget *historyList = new QListWidget(card);
	if (record.games.isEmpty())
	{
		historyList->addItem(QString::fromUtf8(u8"\u6682\u65E0\u5386\u53F2\u5BF9\u5C40\u8BB0\u5F55"));
	}
	else
	{
		for (int i = record.games.size() - 1; i >= 0; --i)
		{
			const GameRecord &game = record.games[i];
			const QString itemText = QString::fromUtf8(
				u8"\u7B2C %1 \u5C40 | %2\n\u7ED3\u679C\uFF1A%3 | \u5148\u624B\uFF1A%4 | \u603B\u624B\u6570\uFF1A%5\n\u843D\u5B50\u9884\u89C8\uFF1A%6")
				.arg(i + 1)
				.arg(game.finishedAt.isEmpty() ? QString::fromUtf8(u8"\u65F6\u95F4\u672A\u8BB0\u5F55") : game.finishedAt)
				.arg(ResultText(game.playerWon))
				.arg(StarterText(game.playerStarted))
				.arg(game.moveCount)
				.arg(MovesPreview(game.moves));
			historyList->addItem(itemText);
		}
	}
	cardLayout->addWidget(historyList, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
	QPushButton *okButton = buttonBox->addButton(QString::fromUtf8(u8"\u5173\u95ED"), QDialogButtonBox::AcceptRole);
	QObject::connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
	cardLayout->addWidget(buttonBox);
}
