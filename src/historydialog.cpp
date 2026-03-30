#include "historydialog.h"

#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
	QString StarterText(bool playerStarted)
	{
		return playerStarted ? QString::fromUtf8(u8"我先手") : QString::fromUtf8(u8"AI 先手");
	}

	QString ResultText(bool playerWon)
	{
		return playerWon ? QString::fromUtf8(u8"胜利") : QString::fromUtf8(u8"失利");
	}

	QString MoveText(const MoveRecord &move)
	{
		return QString::fromUtf8(u8"(%1,%2)").arg(move.row + 1).arg(move.col + 1);
	}

	QString MovesPreview(const QVector<MoveRecord> &moves)
	{
		if (moves.isEmpty())
			return QString::fromUtf8(u8"暂无落子序列");

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
	, m_pHistoryList(nullptr)
	, m_pDeleteButton(nullptr)
	, m_pReplayButton(nullptr)
	, m_pSearchEdit(nullptr)
	, m_pResultFilter(nullptr)
	, m_pDateFrom(nullptr)
	, m_pDateTo(nullptr)
	, m_record(record)
{
	setModal(true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowTitle(QString::fromUtf8(u8"历史对局"));
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
		"QListWidget::item:selected {"
		"background-color: rgba(200, 218, 240, 228);"
		"color: rgb(30, 50, 80);"
		"}"
		"QPushButton#closeBtn {"
		"min-height: 46px;"
		"padding: 10px 22px;"
		"border: none;"
		"border-radius: 14px;"
		"font-size: 18px;"
		"font-weight: 700;"
		"color: white;"
		"background-color: rgb(63, 84, 117);"
		"}"
		"QPushButton#deleteBtn {"
		"min-height: 46px;"
		"padding: 10px 22px;"
		"border: none;"
		"border-radius: 14px;"
		"font-size: 18px;"
		"font-weight: 700;"
		"color: white;"
		"background-color: rgb(200, 80, 80);"
		"}"
		"QPushButton#deleteBtn:disabled {"
		"background-color: rgb(180, 180, 180);"
		"}"
		"QPushButton#deleteBtn:hover:!disabled {"
		"background-color: rgb(220, 90, 90);"
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

	QLabel *title = new QLabel(QString::fromUtf8(u8"%1 的历史对局").arg(userName), card);
	title->setObjectName("title");
	cardLayout->addWidget(title);

	QLabel *subtitle = new QLabel(
		QString::fromUtf8(u8"累计 %1 局 | 我 %2 胜 | AI %3 胜 | 默认开局：%4")
			.arg(record.games.size())
			.arg(record.wins)
			.arg(record.losses)
			.arg(StarterText(record.preferredStarter == "player")),
		card);
	subtitle->setObjectName("subtitle");
	subtitle->setWordWrap(true);
	cardLayout->addWidget(subtitle);

	// 筛选行：关键词搜索 + 胜负筛选 + 日期范围。
	QHBoxLayout *filterLayout = new QHBoxLayout;
	filterLayout->setSpacing(8);

	m_pSearchEdit = new QLineEdit(card);
	m_pSearchEdit->setPlaceholderText(QString::fromUtf8(u8"搜索（日期/手数等）"));
	m_pSearchEdit->setMinimumWidth(180);
	filterLayout->addWidget(m_pSearchEdit);

	m_pResultFilter = new QComboBox(card);
	m_pResultFilter->addItem(QString::fromUtf8(u8"全部"),   -1);
	m_pResultFilter->addItem(QString::fromUtf8(u8"胜利"),    1);
	m_pResultFilter->addItem(QString::fromUtf8(u8"失利"),    0);
	filterLayout->addWidget(m_pResultFilter);

	filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"从"), card));
	m_pDateFrom = new QDateEdit(card);
	m_pDateFrom->setDisplayFormat("yyyy-MM-dd");
	m_pDateFrom->setDate(QDate(2020, 1, 1));
	m_pDateFrom->setCalendarPopup(true);
	filterLayout->addWidget(m_pDateFrom);

	filterLayout->addWidget(new QLabel(QString::fromUtf8(u8"至"), card));
	m_pDateTo = new QDateEdit(card);
	m_pDateTo->setDisplayFormat("yyyy-MM-dd");
	m_pDateTo->setDate(QDate::currentDate());
	m_pDateTo->setCalendarPopup(true);
	filterLayout->addWidget(m_pDateTo);
	filterLayout->addStretch();

	cardLayout->addLayout(filterLayout);

	// 对局列表：支持多选（ExtendedSelection = Ctrl+点击多选 / Shift+点击范围选）。
	m_pHistoryList = new QListWidget(card);
	m_pHistoryList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	// 初始填充（无筛选条件）
	applyFilter();
	cardLayout->addWidget(m_pHistoryList, 1);

	// 底部按钮栏：回放按钮 + 删除按钮 + 关闭按钮。
	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);

	m_pReplayButton = new QPushButton(QString::fromUtf8(u8"▶ 回放"), card);
	m_pReplayButton->setObjectName("replayBtn");
	m_pReplayButton->setEnabled(false);
	buttonBox->addButton(m_pReplayButton, QDialogButtonBox::ActionRole);

	m_pDeleteButton = new QPushButton(QString::fromUtf8(u8"删除选中"), card);
	m_pDeleteButton->setObjectName("deleteBtn");
	m_pDeleteButton->setEnabled(false);
	buttonBox->addButton(m_pDeleteButton, QDialogButtonBox::ActionRole);

	QPushButton *closeButton = buttonBox->addButton(QString::fromUtf8(u8"关闭"), QDialogButtonBox::AcceptRole);
	closeButton->setObjectName("closeBtn");
	QObject::connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

	cardLayout->addWidget(buttonBox);

	// 信号连接：选择变化时更新按钮状态，各按钮点击时执行对应流程。
	connect(m_pHistoryList, &QListWidget::itemSelectionChanged, this, &HistoryDialog::onSelectionChanged);
	connect(m_pDeleteButton, &QPushButton::clicked, this, &HistoryDialog::onDeleteClicked);
	connect(m_pReplayButton, &QPushButton::clicked, this, &HistoryDialog::onReplayClicked);
	// 筛选条件变化时重新过滤
	connect(m_pSearchEdit, &QLineEdit::textChanged, this, &HistoryDialog::onFilterChanged);
	connect(m_pResultFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryDialog::onFilterChanged);
	connect(m_pDateFrom, &QDateEdit::dateChanged, this, &HistoryDialog::onFilterChanged);
	connect(m_pDateTo, &QDateEdit::dateChanged, this, &HistoryDialog::onFilterChanged);
}

void HistoryDialog::onFilterChanged()
{
	applyFilter();
}

void HistoryDialog::applyFilter()
{
	m_pHistoryList->clear();
	m_gameIndices.clear();

	const QString keyword = m_pSearchEdit ? m_pSearchEdit->text().trimmed().toLower() : QString();
	const int resultFilter = m_pResultFilter ? m_pResultFilter->currentData().toInt() : -1;
	const QDate dateFrom = m_pDateFrom ? m_pDateFrom->date() : QDate(2020, 1, 1);
	const QDate dateTo   = m_pDateTo   ? m_pDateTo->date()   : QDate::currentDate();

	if (m_record.games.isEmpty()) {
		m_pHistoryList->addItem(QString::fromUtf8(u8"暂无历史对局记录"));
		m_pHistoryList->setSelectionMode(QAbstractItemView::NoSelection);
		return;
	}

	m_pHistoryList->setSelectionMode(QAbstractItemView::ExtendedSelection);

	for (int i = m_record.games.size() - 1; i >= 0; --i) {
		const GameRecord &game = m_record.games[i];

		// 胜负筛选
		if (resultFilter == 1 && !game.playerWon) continue;
		if (resultFilter == 0 &&  game.playerWon) continue;

		// 日期范围筛选
		if (!game.finishedAt.isEmpty()) {
			const QDate gameDate = QDate::fromString(game.finishedAt.left(10), "yyyy-MM-dd");
			if (gameDate.isValid() && (gameDate < dateFrom || gameDate > dateTo)) continue;
		}

		const QString itemText = QString::fromUtf8(
			u8"第 %1 局 | %2\n结果：%3 | 先手：%4 | 总手数：%5\n落子预览：%6")
			.arg(i + 1)
			.arg(game.finishedAt.isEmpty() ? QString::fromUtf8(u8"时间未记录") : game.finishedAt)
			.arg(ResultText(game.playerWon))
			.arg(StarterText(game.playerStarted))
			.arg(game.moveCount)
			.arg(MovesPreview(game.moves));

		// 关键词搜索（在显示文本中查找）
		if (!keyword.isEmpty() && !itemText.toLower().contains(keyword)) continue;

		m_pHistoryList->addItem(itemText);
		m_gameIndices.push_back(i);
	}

	if (m_pHistoryList->count() == 0) {
		m_pHistoryList->addItem(QString::fromUtf8(u8"无匹配的对局记录"));
		m_pHistoryList->setSelectionMode(QAbstractItemView::NoSelection);
	}
}

void HistoryDialog::onSelectionChanged()
{
	const bool hasSingle = m_pHistoryList->selectedItems().size() == 1;
	const bool hasAny    = !m_pHistoryList->selectedItems().isEmpty();
	// 有选中项时启用删除按钮；仅选中单项时启用回放按钮。
	m_pDeleteButton->setEnabled(hasAny);
	m_pReplayButton->setEnabled(hasSingle);
}

void HistoryDialog::onReplayClicked()
{
	// 取单选项的原始 games 索引，发出回放信号后关闭对话框。
	const QList<QListWidgetItem *> selected = m_pHistoryList->selectedItems();
	if (selected.size() != 1)
		return;
	const int row = m_pHistoryList->row(selected.first());
	if (row < 0 || row >= m_gameIndices.size())
		return;
	emit replayRequested(m_gameIndices[row]);
	accept();
}

void HistoryDialog::onDeleteClicked()
{
	// 收集所有选中项对应的原始 games 数组索引。
	const QList<QListWidgetItem *> selected = m_pHistoryList->selectedItems();
	if (selected.isEmpty())
		return;

	QVector<int> indices;
	indices.reserve(selected.size());
	for (QListWidgetItem *item : selected)
	{
		const int row = m_pHistoryList->row(item);
		if (row >= 0 && row < m_gameIndices.size())
			indices.push_back(m_gameIndices[row]);
	}

	// 弹出确认对话框，防止误删。
	const QString msg = QString::fromUtf8(u8"确定删除选中的 %1 条对局记录？此操作不可撤销。").arg(indices.size());
	QMessageBox::StandardButton result = QMessageBox::warning(
		this,
		QString::fromUtf8(u8"确认删除"),
		msg,
		QMessageBox::Yes | QMessageBox::No);

	if (result == QMessageBox::Yes)
	{
		// 发出删除信号，由调用方（Backgammon）执行实际的数据删除和持久化。
		emit deletedIndices(indices);
		// 关闭对话框。
		accept();
	}
}

