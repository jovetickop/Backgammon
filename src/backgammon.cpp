#include "backgammon.h"

#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPen>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <cmath>

#include "ComputerMove.h"
#include "Evaluation.h"
#include "historydialog.h"
#include "judgeWinner.h"
#include "resultdialog.h"
#include "winratechart.h"

using namespace std;

namespace
{
	const int kBoardSize = 15;
	const int kBoardCenter = 7;
	const int kGridSize = 52;
	const int kPieceSize = 44;
	const int kPieceRadius = kPieceSize / 2;
	const int kLineMin = -kBoardCenter * kGridSize;
	const int kLineMax = kBoardCenter * kGridSize;
	const int kScenePadding = kGridSize;

	int BoardToScene(int index)
	{
		return (index - kBoardCenter) * kGridSize;
	}

	int SceneToBoard(double pos)
	{
		return static_cast<int>(std::lround(pos / kGridSize)) + kBoardCenter;
	}

	double ScoreToAiWinRate(int score)
	{
		const double maxScore = static_cast<double>(FIVE);
		double boundedScore = score;
		if (boundedScore > maxScore)
			boundedScore = maxScore;
		if (boundedScore < -maxScore)
			boundedScore = -maxScore;

		const double normalized = boundedScore == 0.0
			? 0.0
			: std::log10(std::fabs(boundedScore) + 1.0) / std::log10(maxScore + 1.0);
		const double offset = normalized * 49.0;
		return 50.0 + (boundedScore > 0.0 ? offset : -offset);
	}

	QString StarterText(bool playerStarts)
	{
		return playerStarts
			? QString::fromUtf8(u8"\u6211\u5148\u624B\uFF08\u9ED1\u68CB\uFF09")
			: QString::fromUtf8(u8"AI \u5148\u624B\uFF08\u767D\u68CB\uFF09");
	}

	void ShowAiLogicDialog(QWidget *parent, int searchDepth)
	{
		QDialog dialog(parent);
		dialog.setModal(true);
		dialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		dialog.setWindowTitle(QString::fromUtf8(u8"AI \u8FD0\u884C\u903B\u8F91"));
		dialog.setFixedSize(620, 500);
		dialog.setStyleSheet(
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
			"QLabel#section {"
			"background-color: rgba(255,255,255,188);"
			"border: 1px solid rgba(221,228,236,220);"
			"border-radius: 16px;"
			"padding: 14px 16px;"
			"color: rgb(56, 69, 89);"
			"font-size: 17px;"
			"font-weight: 600;"
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

		QVBoxLayout *rootLayout = new QVBoxLayout(&dialog);
		rootLayout->setContentsMargins(18, 18, 18, 18);

		QFrame *card = new QFrame(&dialog);
		card->setObjectName("card");
		rootLayout->addWidget(card);

		QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(&dialog);
		shadow->setBlurRadius(34);
		shadow->setOffset(0, 12);
		shadow->setColor(QColor(65, 82, 106, 80));
		card->setGraphicsEffect(shadow);

		QVBoxLayout *cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(28, 28, 28, 24);
		cardLayout->setSpacing(14);

		QLabel *title = new QLabel(QString::fromUtf8(u8"AI \u51B3\u7B56\u8BF4\u660E"), card);
		title->setObjectName("title");
		cardLayout->addWidget(title);

		QLabel *subtitle = new QLabel(
			QString::fromUtf8(u8"\u5F53\u524D\u641C\u7D22\u6DF1\u5EA6\uFF1A%1 \u5C42\u3002\u8FD9\u91CC\u7684\u201C\u5C42\u201D\u8868\u793A AI \u4E0E\u73A9\u5BB6\u8F6E\u6D41\u5411\u540E\u63A8\u6F14\u7684\u641C\u7D22\u5C42\u7EA7\u3002").arg(searchDepth),
			card);
		subtitle->setObjectName("subtitle");
		subtitle->setWordWrap(true);
		cardLayout->addWidget(subtitle);

		QLabel *logicOne = new QLabel(
			QString::fromUtf8(u8"1. \u5019\u9009\u70B9\u751F\u6210\n\u53EA\u8003\u8651\u5DF2\u6709\u68CB\u5B50\u516B\u90BB\u57DF\u9644\u8FD1\u7684\u7A7A\u4F4D\uFF0C\u907F\u514D\u628A\u65F6\u95F4\u6D6A\u8D39\u5728\u79BB\u6218\u573A\u5F88\u8FDC\u7684\u4F4D\u7F6E\u3002"),
			card);
		logicOne->setObjectName("section");
		logicOne->setWordWrap(true);
		cardLayout->addWidget(logicOne);

		QLabel *logicTwo = new QLabel(
			QString::fromUtf8(u8"2. \u641C\u7D22\u65B9\u5F0F\nAI \u4F7F\u7528\u6781\u5927\u6781\u5C0F\u641C\u7D22\uFF1AAI \u843D\u5B50\u65F6\u5C3D\u91CF\u8BA9\u5C40\u9762\u5206\u6570\u66F4\u9AD8\uFF0C\u73A9\u5BB6\u56DE\u5408\u5219\u5047\u8BBE\u4F60\u4F1A\u9009\u62E9\u6700\u538B\u5236 AI \u7684\u5E94\u624B\u3002"),
			card);
		logicTwo->setObjectName("section");
		logicTwo->setWordWrap(true);
		cardLayout->addWidget(logicTwo);

		QLabel *logicThree = new QLabel(
			QString::fromUtf8(u8"3. \u526A\u679D\u4F18\u5316\n\u641C\u7D22\u8FC7\u7A0B\u4E2D\u4F7F\u7528 alpha-beta \u526A\u679D\uFF0C\u63D0\u524D\u622A\u65AD\u5DF2\u7ECF\u4E0D\u53EF\u80FD\u66F4\u4F18\u7684\u5206\u652F\uFF0C\u63D0\u9AD8\u601D\u8003\u901F\u5EA6\u3002"),
			card);
		logicThree->setObjectName("section");
		logicThree->setWordWrap(true);
		cardLayout->addWidget(logicThree);

		QLabel *logicFour = new QLabel(
			QString::fromUtf8(u8"4. \u5C40\u9762\u8BC4\u4F30\n\u8BC4\u4F30\u5668\u4F1A\u7EDF\u8BA1\u6D3B\u4E00\u3001\u6D3B\u4E8C\u3001\u6D3B\u4E09\u3001\u6D3B\u56DB\u3001\u51B2\u56DB\u3001\u4E94\u8FDE\u7B49\u68CB\u578B\u5206\u503C\uFF0C\u6700\u540E\u7528\u201C\u767D\u65B9\u5206 - \u9ED1\u65B9\u5206\u201D\u5F97\u5230\u5F53\u524D\u5C40\u9762\u4F18\u52A3\u3002"),
			card);
		logicFour->setObjectName("section");
		logicFour->setWordWrap(true);
		cardLayout->addWidget(logicFour);

		QLabel *logicFive = new QLabel(
			QString::fromUtf8(u8"5. \u6700\u7EC8\u843D\u5B50\nAI \u4F1A\u5728\u641C\u7D22\u5230\u7684\u6700\u9AD8\u5206\u5019\u9009\u70B9\u4E2D\u968F\u673A\u9009\u4E00\u4E2A\uFF0C\u907F\u514D\u6BCF\u6B21\u90FD\u8D70\u5B8C\u5168\u76F8\u540C\u7684\u56FA\u5B9A\u5957\u8DEF\u3002"),
			card);
		logicFive->setObjectName("section");
		logicFive->setWordWrap(true);
		cardLayout->addWidget(logicFive);

		cardLayout->addStretch(1);

		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
		QPushButton *okButton = buttonBox->addButton(QString::fromUtf8(u8"\u77E5\u9053\u4E86"), QDialogButtonBox::AcceptRole);
		QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
		cardLayout->addWidget(buttonBox);

		dialog.exec();
	}
}

Backgammon::Backgammon(PlayerStatsStore *statsStore, const PlayerRecord &playerRecord, QWidget *parent)
	: QMainWindow(parent)
	, m_pGraphicsScene(0)
	, m_pLastAiPiece(0)
	, m_pWinRateChart(0)
	, m_pStatsStore(statsStore)
	, m_playerRecord(playerRecord)
	, m_sCurrentUser(playerRecord.displayName)
	, m_bStarted(false)
	, m_bPlayerStarts(playerRecord.preferredStarter == "player")
	, m_pJugdeWinner(0)
	, m_pEvaluation(0)
	, m_nPlayerWins(playerRecord.wins)
	, m_nAiWins(playerRecord.losses)
	, m_nFinishedGames(playerRecord.games.isEmpty() ? playerRecord.wins + playerRecord.losses : playerRecord.games.size())
	, m_nMoveCount(0)
	, m_nPlayerWinRate(50.0)
	, m_nAiWinRate(50.0)
	, m_nDeep(8)
{
	ui.setupUi(this);

	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
			m_arrBoard[i][j] = NONE;
	}

	m_pGraphicsScene = new QGraphicsScene;
	m_pGraphicsScene->setSceneRect(kLineMin - kScenePadding, kLineMin - kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding);

	setStyleSheet(
		"QMainWindow#BackgammonClass {"
		"background: qradialgradient(cx:0.2, cy:0.15, radius:1.25, stop:0 rgba(255,255,255,230), stop:1 rgba(203,217,230,230));"
		"}"
		"QWidget#left_widget {"
		"background-color: rgba(255,255,255,112);"
		"border: 1px solid rgba(255,255,255,195);"
		"border-radius: 20px;"
		"}"
		"QPushButton#startButton, QPushButton#historyButton {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 12px;"
		"padding: 14px 18px;"
		"font-size: 20px;"
		"font-weight: 600;"
		"}"
		"QPushButton#startButton:hover, QPushButton#historyButton:hover {"
		"background-color: rgba(255,255,255,228);"
		"}"
		"QPushButton#startButton:pressed, QPushButton#historyButton:pressed {"
		"background-color: rgba(236,242,249,236);"
		"}"
		"QComboBox#starterComboBox {"
		"min-height: 56px;"
		"padding: 0 16px;"
		"font-size: 18px;"
		"font-weight: 600;"
		"border: 1px solid rgba(190,203,220,220);"
		"border-radius: 14px;"
		"background-color: rgba(255,255,255,210);"
		"color: rgb(47, 58, 76);"
		"}"
		"QComboBox#starterComboBox::drop-down {"
		"width: 40px;"
		"border: none;"
		"}"
		"QPushButton#aiInfoButton {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 34px;"
		"font-size: 28px;"
		"font-weight: 700;"
		"color: rgb(63, 84, 117);"
		"}"
		"QPushButton#aiInfoButton:hover {"
		"background-color: rgba(255,255,255,232);"
		"}"
		"QPushButton#aiInfoButton:pressed {"
		"background-color: rgba(236,242,249,236);"
		"}"
		"QLabel#starterTitleLabel,"
		"QLabel#currentUserTitleLabel,"
		"QLabel#historyTitleLabel,"
		"QLabel#statsTitleLabel {"
		"color: rgb(58,70,90);"
		"font-size: 22px;"
		"font-weight: 700;"
		"padding-top: 8px;"
		"}"
		"QLabel#currentUserLabel,"
		"QLabel#historyGamesLabel, QLabel#historyPlayerRateLabel, QLabel#historyAiRateLabel,"
		"QLabel#gamesLabel, QLabel#playerRateLabel, QLabel#aiRateLabel {"
		"color: rgb(74,86,105);"
		"background-color: rgba(255,255,255,148);"
		"border: 1px solid rgba(255,255,255,198);"
		"border-radius: 12px;"
		"padding: 14px 16px;"
		"font-size: 18px;"
		"font-weight: 600;"
		"}"
		"QWidget#chartHost {"
		"background-color: rgba(255,255,255,128);"
		"border: 1px solid rgba(255,255,255,195);"
		"border-radius: 16px;"
		"}"
		"QGraphicsView#graphicsView {"
		"background: rgba(255,255,255,78);"
		"border: 1px solid rgba(255,255,255,175);"
		"border-radius: 18px;"
		"}");

	ui.starterComboBox->addItem(QString::fromUtf8(u8"\u6211\u5148\u624B\uFF08\u9ED1\u68CB\uFF09"), "player");
	ui.starterComboBox->addItem(QString::fromUtf8(u8"AI \u5148\u624B\uFF08\u767D\u68CB\uFF09"), "ai");
	const int starterIndex = m_bPlayerStarts ? 0 : 1;
	ui.starterComboBox->setCurrentIndex(starterIndex);

	ui.graphicsView->setBackgroundBrush(QColor(230, 196, 143, 225));
	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);
	ui.graphicsView->setScene(m_pGraphicsScene);
	ui.graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	ui.graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);

	QGraphicsDropShadowEffect *leftShadow = new QGraphicsDropShadowEffect(this);
	leftShadow->setBlurRadius(34);
	leftShadow->setOffset(0, 10);
	leftShadow->setColor(QColor(74, 91, 116, 88));
	ui.left_widget->setGraphicsEffect(leftShadow);

	QGraphicsDropShadowEffect *boardShadow = new QGraphicsDropShadowEffect(this);
	boardShadow->setBlurRadius(26);
	boardShadow->setOffset(0, 8);
	boardShadow->setColor(QColor(63, 82, 105, 76));
	ui.graphicsView->setGraphicsEffect(boardShadow);

	QVBoxLayout *chartLayout = new QVBoxLayout(ui.chartHost);
	chartLayout->setContentsMargins(10, 10, 10, 10);
	chartLayout->setSpacing(0);

	m_pWinRateChart = new WinRateChart(ui.chartHost);
	chartLayout->addWidget(m_pWinRateChart);

	DrawBoard();
	UpdateStatsPanel();

	connect(ui.startButton, SIGNAL(clicked()), this, SLOT(slotStartBtnClicked()));
	connect(ui.historyButton, SIGNAL(clicked()), this, SLOT(slotHistoryBtnClicked()));
	connect(ui.starterComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotStarterChanged(int)));
	connect(ui.aiInfoButton, &QPushButton::clicked, this, [this]() { ShowAiLogicDialog(this, m_nDeep); });
	ui.startButton->setChecked(false);
	m_pJugdeWinner = new judgeWinner();
	m_pEvaluation = new Evaluation();
}

Backgammon::~Backgammon()
{
	delete m_pEvaluation;
	delete m_pJugdeWinner;
}

void Backgammon::DrawBoard()
{
	QPen gridPen(QColor(82, 56, 32, 185));
	gridPen.setWidth(2);
	for (int i = 0; i < kBoardSize; ++i)
	{
		const int coord = BoardToScene(i);
		m_pGraphicsScene->addLine(kLineMin, coord, kLineMax, coord, gridPen);
		m_pGraphicsScene->addLine(coord, kLineMin, coord, kLineMax, gridPen);
	}
}

void Backgammon::slotStartBtnClicked()
{
	if (ui.startButton->isChecked())
	{
		ui.startButton->setText(QString::fromUtf8(u8"\u6E05\u9664"));
		ui.starterComboBox->setEnabled(false);
		m_bStarted = true;
		ResetWinRateEstimate();
		if (!m_bPlayerStarts)
			PlaceAiOpeningMove();
		return;
	}

	if (IsBoardClean())
	{
		FinishRoundCleanup();
		return;
	}

	QMessageBox::StandardButton btn = QMessageBox::warning(
		this,
		QString::fromUtf8(u8"\u8B66\u544A"),
		QString::fromUtf8(u8"\u786E\u5B9A\u6E05\u9664\u68CB\u76D8\uFF1F"),
		QMessageBox::Yes | QMessageBox::No);
	if (btn == QMessageBox::Yes)
	{
		FinishRoundCleanup();
		return;
	}

	ui.startButton->setChecked(true);
}

void Backgammon::slotStarterChanged(int)
{
	m_bPlayerStarts = (CurrentStarterPreference() == "player");
	m_playerRecord.preferredStarter = CurrentStarterPreference();
	PersistPlayerRecord();
	UpdateStatsPanel();
}

void Backgammon::slotHistoryBtnClicked()
{
	HistoryDialog(m_sCurrentUser, m_playerRecord, this).exec();
}

bool Backgammon::IsBoardClean()
{
	return m_pGraphicsScene->items().size() <= (kBoardSize * 2);
}

void Backgammon::showEvent(QShowEvent *event)
{
	QMainWindow::showEvent(event);
	UpdateBoardView();
}

void Backgammon::resizeEvent(QResizeEvent *event)
{
	QMainWindow::resizeEvent(event);
	UpdateBoardView();
}

void Backgammon::mousePressEvent(QMouseEvent * event)
{
	if (!m_bStarted)
		return;

	QPoint viewPos = ui.graphicsView->mapFrom(this, event->pos());
	if (!ui.graphicsView->rect().contains(viewPos))
		return;

	QPointF scenePos = ui.graphicsView->mapToScene(viewPos);
	const int row = SceneToBoard(scenePos.x());
	const int col = SceneToBoard(scenePos.y());

	if (row >= kBoardSize || row < 0 || col >= kBoardSize || col < 0)
	{
		QMessageBox::warning(this, QString::fromUtf8(u8"\u8B66\u544A"), QString::fromUtf8(u8"\u65E0\u6CD5\u843D\u5B50\uFF01"));
		return;
	}

	if (m_arrBoard[row][col] != NONE)
	{
		QMessageBox::warning(this, QString::fromUtf8(u8"\u8B66\u544A"), QString::fromUtf8(u8"\u65E0\u6CD5\u843D\u5B50\uFF01"));
		return;
	}

	PlacePlayerMove(row, col);
	UpdateWinRateEstimate();

	if (m_pJugdeWinner->IsWon(BLACK, m_arrBoard))
	{
		RecordGameResult(BLACK);
		ResultDialog(this, true, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		FinishRoundCleanup();
		return;
	}

	ComputerMove* pComputerMove = new ComputerMove();
	pComputerMove->MaxMinSearch(m_arrBoard, m_nDeep);
	const int aiRow = pComputerMove->X();
	const int aiCol = pComputerMove->Y();
	delete pComputerMove;

	PlaceAiMove(aiRow, aiCol);
	UpdateWinRateEstimate();

	if (m_pJugdeWinner->IsWon(WHITE, m_arrBoard))
	{
		RecordGameResult(WHITE);
		ResultDialog(this, false, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		FinishRoundCleanup();
		return;
	}
}

void Backgammon::CleanBoard()
{
	QList<QGraphicsItem*> list = m_pGraphicsScene->items();
	while (!list.isEmpty())
	{
		m_pGraphicsScene->removeItem(list[0]);
		list.removeFirst();
	}

	DrawBoard();

	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
			m_arrBoard[i][j] = NONE;
	}

	ResetWinRateEstimate();
	UpdateBoardView();
}

void Backgammon::RecordGameResult(ePiece winner)
{
	if (winner != BLACK && winner != WHITE)
		return;

	GameRecord game;
	game.finishedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	game.playerWon = (winner == BLACK);
	game.playerStarted = m_bPlayerStarts;
	game.moveCount = m_currentGameMoves.size();
	game.moves = m_currentGameMoves;
	m_playerRecord.games.push_back(game);

	if (winner == BLACK)
		++m_nPlayerWins;
	else
		++m_nAiWins;

	m_nFinishedGames = m_playerRecord.games.size();
	PersistPlayerRecord();
	UpdateStatsPanel();
}

void Backgammon::PersistPlayerRecord()
{
	if (!m_pStatsStore || m_sCurrentUser.trimmed().isEmpty())
		return;

	m_playerRecord.displayName = m_sCurrentUser;
	m_playerRecord.wins = m_nPlayerWins;
	m_playerRecord.losses = m_nAiWins;
	m_playerRecord.preferredStarter = CurrentStarterPreference();
	m_pStatsStore->SaveRecord(m_playerRecord);
	m_pStatsStore->Save();
}

void Backgammon::ResetWinRateEstimate()
{
	m_nMoveCount = 0;
	m_nPlayerWinRate = 50.0;
	m_nAiWinRate = 50.0;
	m_playerRateHistory.clear();
	m_aiRateHistory.clear();
	m_currentGameMoves.clear();
	m_pLastAiPiece = 0;
	UpdateStatsPanel();
}

void Backgammon::UpdateStatsPanel()
{
	ui.currentUserLabel->setText(
		QString::fromUtf8(u8"%1\n\u9ED8\u8BA4\u5F00\u5C40\uFF1A%2")
			.arg(m_sCurrentUser)
			.arg(StarterText(m_bPlayerStarts)));
	ui.historyGamesLabel->setText(QString::fromUtf8(u8"\u7D2F\u8BA1\u5C40\u6570 %1").arg(m_nFinishedGames));
	ui.historyPlayerRateLabel->setText(
		QString::fromUtf8(u8"\u6211 %1 \u80DC | \u80DC\u7387 %2%")
			.arg(m_nPlayerWins)
			.arg(m_nFinishedGames == 0 ? 0 : qRound(m_nPlayerWins * 100.0 / m_nFinishedGames)));
	ui.historyAiRateLabel->setText(
		QString::fromUtf8(u8"AI %1 \u80DC | \u80DC\u7387 %2%")
			.arg(m_nAiWins)
			.arg(m_nFinishedGames == 0 ? 0 : qRound(m_nAiWins * 100.0 / m_nFinishedGames)));
	ui.gamesLabel->setText(QString::fromUtf8(u8"\u5F53\u524D\u624B\u6570 %1").arg(m_nMoveCount));
	ui.playerRateLabel->setText(QString::fromUtf8(u8"\u6211\u7684\u9884\u4F30\u80DC\u7387 %1%").arg(qRound(m_nPlayerWinRate)));
	ui.aiRateLabel->setText(QString::fromUtf8(u8"AI \u9884\u4F30\u80DC\u7387 %1%").arg(qRound(m_nAiWinRate)));
	ui.historyButton->setText(QString::fromUtf8(u8"\u5386\u53F2\u5BF9\u5C40 (%1)").arg(m_playerRecord.games.size()));

	if (m_pWinRateChart)
		m_pWinRateChart->SetSeries(m_playerRateHistory, m_aiRateHistory);
}

void Backgammon::UpdateWinRateEstimate()
{
	if (!m_pEvaluation)
		return;

	if (m_pJugdeWinner->IsWon(WHITE, m_arrBoard))
	{
		m_nAiWinRate = 100.0;
		m_nPlayerWinRate = 0.0;
	}
	else if (m_pJugdeWinner->IsWon(BLACK, m_arrBoard))
	{
		m_nAiWinRate = 0.0;
		m_nPlayerWinRate = 100.0;
	}
	else
	{
		const int score = m_pEvaluation->EvaluateBoard(m_arrBoard);
		m_nAiWinRate = ScoreToAiWinRate(score);
		m_nPlayerWinRate = 100.0 - m_nAiWinRate;
	}

	++m_nMoveCount;
	m_playerRateHistory.push_back(m_nPlayerWinRate);
	m_aiRateHistory.push_back(m_nAiWinRate);
	UpdateStatsPanel();
}

void Backgammon::UpdateBoardView()
{
	if (!ui.graphicsView || !ui.graphicsView->scene())
		return;

	if (ui.graphicsView->viewport()->width() <= 0 || ui.graphicsView->viewport()->height() <= 0)
		return;

	ui.graphicsView->resetTransform();
	ui.graphicsView->fitInView(ui.graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void Backgammon::SetLastAiPiece(QGraphicsEllipseItem *piece)
{
	if (m_pLastAiPiece)
	{
		m_pLastAiPiece->setBrush(QColor(Qt::white));
		m_pLastAiPiece->setPen(QPen(Qt::NoPen));
	}

	m_pLastAiPiece = piece;
	if (!m_pLastAiPiece)
		return;

	QPen highlightPen(QColor(214, 170, 92, 220));
	highlightPen.setWidth(3);
	m_pLastAiPiece->setBrush(QColor(255, 249, 228));
	m_pLastAiPiece->setPen(highlightPen);
}

void Backgammon::AppendMove(int row, int col, ePiece piece)
{
	MoveRecord move;
	move.row = row;
	move.col = col;
	move.piece = piece;
	m_currentGameMoves.push_back(move);
}

void Backgammon::PlaceAiMove(int row, int col)
{
	m_arrBoard[row][col] = WHITE;
	const int x = BoardToScene(row) - kPieceRadius;
	const int y = BoardToScene(col) - kPieceRadius;
	QGraphicsEllipseItem *aiPiece = m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
	SetLastAiPiece(aiPiece);
	AppendMove(row, col, WHITE);
}

void Backgammon::PlacePlayerMove(int row, int col)
{
	m_arrBoard[row][col] = BLACK;
	const int x = BoardToScene(row) - kPieceRadius;
	const int y = BoardToScene(col) - kPieceRadius;
	m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::black));
	AppendMove(row, col, BLACK);
}

void Backgammon::PlaceAiOpeningMove()
{
	PlaceAiMove(kBoardCenter, kBoardCenter);
	UpdateWinRateEstimate();
}

void Backgammon::FinishRoundCleanup()
{
	m_bStarted = false;
	ui.startButton->setText(QString::fromUtf8(u8"\u5F00\u59CB"));
	ui.startButton->setChecked(false);
	ui.starterComboBox->setEnabled(true);
	CleanBoard();
}

QString Backgammon::CurrentStarterPreference() const
{
	return ui.starterComboBox->currentData().toString();
}
