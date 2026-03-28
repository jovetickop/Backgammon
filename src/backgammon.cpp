#include "backgammon.h"
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QPen>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <cmath>
#include "ComputerMove.h"
#include "Evaluation.h"
#include "judgeWinner.h"
#include "playerstatsstore.h"
#include "resultdialog.h"
#include "winratechart.h"

using namespace std;

namespace
{
	// 棋盘与绘制参数统一放在此处，后续调尺寸只改这里。
	const int kBoardSize = 15;
	const int kBoardCenter = 7;
	const int kGridSize = 52;
	const int kPieceSize = 44;
	const int kPieceRadius = kPieceSize / 2;
	const int kLineMin = -kBoardCenter * kGridSize;
	const int kLineMax = kBoardCenter * kGridSize;
	const int kScenePadding = kGridSize;

	// 棋盘数组下标 -> 场景坐标。
	int BoardToScene(int index)
	{
		return (index - kBoardCenter) * kGridSize;
	}

	// 场景坐标 -> 棋盘数组下标（吸附到最近交叉点）。
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

	void ShowAiLogicDialog(QWidget *parent, int searchDepth)
	{
		QDialog dialog(parent);
		dialog.setModal(true);
		dialog.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		dialog.setWindowTitle(QString::fromUtf8("AI 运行逻辑"));
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

		QLabel *title = new QLabel(QString::fromUtf8("AI 决策说明"), card);
		title->setObjectName("title");
		cardLayout->addWidget(title);

		QLabel *subtitle = new QLabel(
			QString::fromUtf8("当前搜索深度：%1 层。这里的“层”表示 AI 与玩家轮流向后推演的搜索层级。").arg(searchDepth),
			card);
		subtitle->setObjectName("subtitle");
		subtitle->setWordWrap(true);
		cardLayout->addWidget(subtitle);

		QLabel *logicOne = new QLabel(
			QString::fromUtf8("1. 候选点生成\n只考虑已有棋子八邻域附近的空位，避免把时间浪费在离战场很远的位置。"),
			card);
		logicOne->setObjectName("section");
		logicOne->setWordWrap(true);
		cardLayout->addWidget(logicOne);

		QLabel *logicTwo = new QLabel(
			QString::fromUtf8("2. 搜索方式\nAI 使用极大极小搜索：AI 落子时尽量让局面分数更高，玩家回合则假设你会选择最压制 AI 的应手。"),
			card);
		logicTwo->setObjectName("section");
		logicTwo->setWordWrap(true);
		cardLayout->addWidget(logicTwo);

		QLabel *logicThree = new QLabel(
			QString::fromUtf8("3. 剪枝优化\n搜索过程中使用 alpha-beta 剪枝，提前截断已经不可能更优的分支，提高思考速度。"),
			card);
		logicThree->setObjectName("section");
		logicThree->setWordWrap(true);
		cardLayout->addWidget(logicThree);

		QLabel *logicFour = new QLabel(
			QString::fromUtf8("4. 局面评估\n评估器会统计活一、活二、活三、活四、冲四、五连等棋型分值，最后用“白方分 - 黑方分”得到当前局面优劣。"),
			card);
		logicFour->setObjectName("section");
		logicFour->setWordWrap(true);
		cardLayout->addWidget(logicFour);

		QLabel *logicFive = new QLabel(
			QString::fromUtf8("5. 最终落子\nAI 会在搜索到的最高分候选点中随机选一个，避免每次都走完全相同的固定套路。"),
			card);
		logicFive->setObjectName("section");
		logicFive->setWordWrap(true);
		cardLayout->addWidget(logicFive);

		cardLayout->addStretch(1);

		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, card);
		QPushButton *okButton = buttonBox->addButton(QString::fromUtf8("知道了"), QDialogButtonBox::AcceptRole);
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
	, m_sCurrentUser(playerRecord.displayName)
	, m_bStarted(false)
	, m_pJugdeWinner(0)
	, m_pEvaluation(0)
	, m_nPlayerWins(playerRecord.wins)
	, m_nAiWins(playerRecord.losses)
	, m_nFinishedGames(playerRecord.wins + playerRecord.losses)
	, m_nMoveCount(0)
	, m_nPlayerWinRate(50.0)
	, m_nAiWinRate(50.0)
	, m_nDeep(8)
{
	ui.setupUi(this);

	// 初始化棋盘数据。
	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
		{
			m_arrBoard[i][j] = NONE;
		}
	}

	m_pGraphicsScene = new QGraphicsScene;
	m_pGraphicsScene->setSceneRect(kLineMin - kScenePadding, kLineMin - kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding,
		(kLineMax - kLineMin) + 2 * kScenePadding);

	// 界面风格：浅色背景 + 左侧磨砂面板 + 按钮状态反馈。
	setStyleSheet(
		"QMainWindow#BackgammonClass {"
		"background: qradialgradient(cx:0.2, cy:0.15, radius:1.25, stop:0 rgba(255,255,255,230), stop:1 rgba(203,217,230,230));"
		"}"
		"QWidget#left_widget {"
		"background-color: rgba(255,255,255,112);"
		"border: 1px solid rgba(255,255,255,195);"
		"border-radius: 20px;"
		"}"
		"QPushButton#startButton {"
		"background-color: rgba(255,255,255,182);"
		"border: 1px solid rgba(120,130,150,110);"
		"border-radius: 12px;"
		"padding: 14px 18px;"
		"font-size: 20px;"
		"font-weight: 600;"
		"}"
		"QPushButton#startButton:hover {"
		"background-color: rgba(255,255,255,228);"
		"}"
		"QPushButton#startButton:pressed {"
		"background-color: rgba(236,242,249,236);"
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
		"}"
	);

	ui.graphicsView->setBackgroundBrush(QColor(230, 196, 143, 225));
	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);
	ui.graphicsView->setScene(m_pGraphicsScene);
	ui.graphicsView->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	ui.graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);

	// 用阴影增强“玻璃卡片”层次感。
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
		int coord = BoardToScene(i);
		m_pGraphicsScene->addLine(kLineMin, coord, kLineMax, coord, gridPen);
		m_pGraphicsScene->addLine(coord, kLineMin, coord, kLineMax, gridPen);
	}
}

void Backgammon::slotStartBtnClicked()
{
	if (ui.startButton->isChecked())
	{
		// 开始游戏时由电脑先手，下在天元位置。
		ui.startButton->setText(QString::fromUtf8(u8"\u6e05\u9664"));
		m_bStarted = true;
		ResetWinRateEstimate();
		int x = BoardToScene(kBoardCenter) - kPieceRadius;
		int y = BoardToScene(kBoardCenter) - kPieceRadius;
		QGraphicsEllipseItem *aiPiece = m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
		SetLastAiPiece(aiPiece);
		m_arrBoard[kBoardCenter][kBoardCenter] = WHITE;
		UpdateWinRateEstimate();
	}
	else
	{
		if (!IsBoardClean())
		{
			QMessageBox::StandardButton btn = QMessageBox::warning(
				this,
				QString::fromUtf8(u8"\u8b66\u544a"),
				QString::fromUtf8(u8"\u786e\u5b9a\u6e05\u9664\u68cb\u76d8\uff1f"),
				QMessageBox::Yes | QMessageBox::No);
			if (btn == QMessageBox::Yes)
			{
				m_bStarted = false;
				ui.startButton->setText(QString::fromUtf8(u8"\u5f00\u59cb"));
				CleanBoard();
				return;
			}
			else
			{
				ui.startButton->setChecked(true);
			}
		}
	}
}

bool Backgammon::IsBoardClean()
{
	// 空盘时仅包含 15 条横线 + 15 条竖线。
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

	// 把主窗口坐标转换为 graphicsView 坐标。
	QPoint viewPos = ui.graphicsView->mapFrom(this, event->pos());
	if (!ui.graphicsView->rect().contains(viewPos))
		return;

	QPointF scenePos = ui.graphicsView->mapToScene(viewPos);
	int nHm = SceneToBoard(scenePos.x());
	int nHn = SceneToBoard(scenePos.y());

	if (nHm >= kBoardSize || nHm < 0 || nHn >= kBoardSize || nHn < 0)
	{
		QMessageBox::warning(this, QString::fromUtf8(u8"\u8b66\u544a"), QString::fromUtf8(u8"\u65e0\u6cd5\u843d\u5b50\uff01"));
		return;
	}

	if (m_arrBoard[nHm][nHn] == NONE)
	{
		int x = BoardToScene(nHm) - kPieceRadius;
		int y = BoardToScene(nHn) - kPieceRadius;
		m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::black));
		m_arrBoard[nHm][nHn] = BLACK;
		UpdateWinRateEstimate();
	}
	else
	{
		QMessageBox::warning(this, QString::fromUtf8(u8"\u8b66\u544a"), QString::fromUtf8(u8"\u65e0\u6cd5\u843d\u5b50\uff01"));
		return;
	}

	// 玩家胜负判断。
	if (m_pJugdeWinner->IsWon(BLACK, m_arrBoard))
	{
		RecordGameResult(BLACK);
		ResultDialog(this, true, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		CleanBoard();
		ui.startButton->setText(QString::fromUtf8(u8"\u5f00\u59cb"));
		ui.startButton->setChecked(false);
		m_bStarted = false;
		return;
	}

	// 电脑搜索并落子。
	ComputerMove* pComputerMove = new ComputerMove();
	pComputerMove->MaxMinSearch(m_arrBoard, m_nDeep);
	int nCm = pComputerMove->X();
	int nCn = pComputerMove->Y();
	delete pComputerMove;

	m_arrBoard[nCm][nCn] = WHITE;

	int cx = BoardToScene(nCm) - kPieceRadius;
	int cy = BoardToScene(nCn) - kPieceRadius;
	QGraphicsEllipseItem *aiPiece = m_pGraphicsScene->addEllipse(cx, cy, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
	SetLastAiPiece(aiPiece);
	UpdateWinRateEstimate();

	if (m_pJugdeWinner->IsWon(WHITE, m_arrBoard))
	{
		RecordGameResult(WHITE);
		ResultDialog(this, false, m_nMoveCount, m_nFinishedGames, m_nPlayerWins, m_nAiWins).exec();
		CleanBoard();
		ui.startButton->setText(QString::fromUtf8(u8"\u5f00\u59cb"));
		ui.startButton->setChecked(false);
		m_bStarted = false;
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

	// 同步清空棋盘数组。
	for (int i = 0; i < kBoardSize; ++i)
	{
		for (int j = 0; j < kBoardSize; ++j)
		{
			m_arrBoard[i][j] = NONE;
		}
	}
	ResetWinRateEstimate();
	UpdateBoardView();
}

void Backgammon::RecordGameResult(ePiece winner)
{
	if (winner == BLACK)
	{
		++m_nPlayerWins;
	}
	else if (winner == WHITE)
	{
		++m_nAiWins;
	}
	else
	{
		return;
	}

	++m_nFinishedGames;
	PersistPlayerRecord();
	UpdateStatsPanel();
}

void Backgammon::PersistPlayerRecord()
{
	if (!m_pStatsStore || m_sCurrentUser.trimmed().isEmpty())
		return;

	PlayerRecord record;
	record.displayName = m_sCurrentUser;
	record.wins = m_nPlayerWins;
	record.losses = m_nAiWins;
	m_pStatsStore->SaveRecord(record);
	m_pStatsStore->Save();
}

void Backgammon::ResetWinRateEstimate()
{
	m_nMoveCount = 0;
	m_nPlayerWinRate = 50.0;
	m_nAiWinRate = 50.0;
	m_playerRateHistory.clear();
	m_aiRateHistory.clear();
	m_pLastAiPiece = 0;
	UpdateStatsPanel();
}

void Backgammon::UpdateStatsPanel()
{
	ui.currentUserLabel->setText(m_sCurrentUser);
	ui.historyGamesLabel->setText(QString::fromUtf8("累计局数 %1").arg(m_nFinishedGames));
	ui.historyPlayerRateLabel->setText(QString::fromUtf8("我 %1 胜 | 胜率 %2%").arg(m_nPlayerWins).arg(m_nFinishedGames == 0 ? 0 : qRound(m_nPlayerWins * 100.0 / m_nFinishedGames)));
	ui.historyAiRateLabel->setText(QString::fromUtf8("AI %1 胜 | 胜率 %2%").arg(m_nAiWins).arg(m_nFinishedGames == 0 ? 0 : qRound(m_nAiWins * 100.0 / m_nFinishedGames)));
	ui.gamesLabel->setText(QString::fromUtf8("当前手数 %1").arg(m_nMoveCount));
	ui.playerRateLabel->setText(QString::fromUtf8("我的预估胜率 %1%").arg(qRound(m_nPlayerWinRate)));
	ui.aiRateLabel->setText(QString::fromUtf8("AI 预估胜率 %1%").arg(qRound(m_nAiWinRate)));

	if (m_pWinRateChart)
	{
		m_pWinRateChart->SetSeries(m_playerRateHistory, m_aiRateHistory);
	}
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
