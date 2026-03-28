#include "backgammon.h"
#include <QGraphicsScene>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGraphicsView>
#include <QPen>
#include <cmath>
#include "ComputerMove.h"
#include "judgeWinner.h"

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
}

Backgammon::Backgammon(QWidget *parent)
	: QMainWindow(parent)
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
		"background: qradialgradient(cx:0.2, cy:0.15, radius:1.2, stop:0 rgba(255,255,255,210), stop:1 rgba(216,226,236,220));"
		"}"
		"QWidget#left_widget {"
		"background-color: rgba(255,255,255,125);"
		"border: 1px solid rgba(255,255,255,180);"
		"border-radius: 18px;"
		"}"
		"QPushButton#startButton {"
		"background-color: rgba(255,255,255,170);"
		"border: 1px solid rgba(120,130,150,120);"
		"border-radius: 12px;"
		"padding: 10px 14px;"
		"font-size: 16px;"
		"font-weight: 600;"
		"}"
		"QPushButton#startButton:hover {"
		"background-color: rgba(255,255,255,220);"
		"}"
		"QPushButton#startButton:pressed {"
		"background-color: rgba(238,244,250,230);"
		"}"
		"QGraphicsView#graphicsView {"
		"background: rgba(255,255,255,85);"
		"border: 1px solid rgba(255,255,255,160);"
		"border-radius: 16px;"
		"}"
	);

	ui.graphicsView->setBackgroundBrush(QColor(230, 196, 143, 225));
	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);
	ui.graphicsView->setScene(m_pGraphicsScene);

	DrawBoard();

	connect(ui.startButton, SIGNAL(clicked()), this, SLOT(slotStartBtnClicked()));
	ui.startButton->setChecked(false);
	m_bStarted = false;
	m_pJugdeWinner = new judgeWinner();

	m_nDeep = 8;
}

Backgammon::~Backgammon()
{
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
		int x = BoardToScene(kBoardCenter) - kPieceRadius;
		int y = BoardToScene(kBoardCenter) - kPieceRadius;
		m_pGraphicsScene->addEllipse(x, y, kPieceSize, kPieceSize, QPen(Qt::NoPen), QColor(Qt::white));
		m_arrBoard[kBoardCenter][kBoardCenter] = WHITE;
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
	}
	else
	{
		QMessageBox::warning(this, QString::fromUtf8(u8"\u8b66\u544a"), QString::fromUtf8(u8"\u65e0\u6cd5\u843d\u5b50\uff01"));
		return;
	}

	// 玩家胜负判断。
	if (m_pJugdeWinner->IsWon(BLACK, m_arrBoard))
	{
		QMessageBox::information(this, QString::fromUtf8(u8"Winner"), QString::fromUtf8(u8"\u4f60\u6218\u80dc\u4e86\u8ba1\u7b97\u673a\uff01"));
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

	if (m_pJugdeWinner->IsWon(WHITE, m_arrBoard))
	{
		QMessageBox::information(this, QString::fromUtf8(u8"Winner"), QString::fromUtf8(u8"\u8ba1\u7b97\u673a\u53d6\u5f97\u80dc\u5229\uff01"));
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
}
