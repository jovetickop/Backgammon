#include "backgammon.h"
#include <QGraphicsScene>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>
#include <QShowEvent>
#include <QPen>
#include <QVBoxLayout>
#include <cmath>
#include "ComputerMove.h"
#include "Evaluation.h"
#include "judgeWinner.h"
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
}

Backgammon::Backgammon(QWidget *parent)
	: QMainWindow(parent)
	, m_pGraphicsScene(0)
	, m_pWinRateChart(0)
	, m_bStarted(false)
	, m_pJugdeWinner(0)
	, m_pEvaluation(0)
	, m_nPlayerWins(0)
	, m_nAiWins(0)
	, m_nFinishedGames(0)
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
		"QLabel#historyTitleLabel,"
		"QLabel#statsTitleLabel {"
		"color: rgb(58,70,90);"
		"font-size: 22px;"
		"font-weight: 700;"
		"padding-top: 8px;"
		"}"
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
		m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
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
	m_pGraphicsScene->addEllipse(cx, cy, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
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
	UpdateStatsPanel();
}

void Backgammon::ResetWinRateEstimate()
{
	m_nMoveCount = 0;
	m_nPlayerWinRate = 50.0;
	m_nAiWinRate = 50.0;
	m_playerRateHistory.clear();
	m_aiRateHistory.clear();
	UpdateStatsPanel();
}

void Backgammon::UpdateStatsPanel()
{
	ui.historyGamesLabel->setText(QString::fromUtf8("累计局数 %1").arg(m_nFinishedGames));
	ui.historyPlayerRateLabel->setText(QString::fromUtf8("我的历史胜率 %1%").arg(m_nFinishedGames == 0 ? 0 : qRound(m_nPlayerWins * 100.0 / m_nFinishedGames)));
	ui.historyAiRateLabel->setText(QString::fromUtf8("AI 历史胜率 %1%").arg(m_nFinishedGames == 0 ? 0 : qRound(m_nAiWins * 100.0 / m_nFinishedGames)));
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
