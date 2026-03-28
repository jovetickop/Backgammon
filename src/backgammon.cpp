#include "backgammon.h"
#include<QGraphicsScene>
#include<QMessageBox>
#include<QMouseEvent>
#include<QGraphicsView>
#include"ComputerMove.h"
#include"judgeWinner.h"
using namespace std;

Backgammon::Backgammon(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	for(int i = 0; i< 15; ++i)
	{
		for(int j = 0; j< 15; ++j)
		{
			m_arrBoard[i][j] = NONE;
		}
	}

	m_pGraphicsScene = new QGraphicsScene;
	m_pGraphicsScene->setSceneRect(-300, -300, 600, 600);

	ui.graphicsView->setBackgroundBrush(QColor(237,191,118));    //设置棋盘颜色
	ui.graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.graphicsView->setRenderHint(QPainter::Antialiasing);  //反走样
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
	for(int i = 0; i< 15; ++i)
	{
		m_pGraphicsScene->addLine(-300, -300+40*i, 300, -300+40*i);  //添加横线
		m_pGraphicsScene->addLine(-300+40*i, -300, -300+40*i, 300);   //添加竖线
	}
}

void Backgammon::slotStartBtnClicked()
{
	if(ui.startButton->isChecked()) 
	{
		ui.startButton->setText(QString::fromLocal8Bit("清除"));
		m_bStarted = true;
		m_pGraphicsScene->addEllipse(-40, -40, 35, 35, QPen(Qt::NoPen), QColor(Qt::white));
		m_arrBoard[7][7] = WHITE;
	}
	else
	{
		if(!IsBoardClean())
		{
			QMessageBox::StandardButton btn = QMessageBox::warning(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("确定清除棋盘？"), QMessageBox::Yes | QMessageBox::No);
			if(btn == QMessageBox::Yes)
			{			
				m_bStarted = false;
				ui.startButton->setText(QString::fromLocal8Bit("开始"));
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
	return m_pGraphicsScene->items().size() <=30 ? true:false;
}

void Backgammon::mousePressEvent(QMouseEvent * event)
{
	if(!m_bStarted)
		return;
	QPoint viewPos = event->pos();
	if(event->pos().x()<100)
		return;

	QPointF scenePos = ui.graphicsView->mapToScene(viewPos.x()-100, viewPos.y());

	int nHx, nHy;

	if(scenePos.x()<0)
	{
		
		nHx = scenePos.x()/40-1;
	}
	else
	{
		nHx = scenePos.x()/40;
	}
	if(scenePos.y()<0)
	{
		
		nHy = scenePos.y()/40-1;
	}
	else
	{
		nHy = scenePos.y()/40;
	}

	int nHm,nHn;
	nHm = nHx+8;
	nHn = nHy+8;

	if(nHm >= 15 || nHm < 0 || nHn >=15 || nHn <0)
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法落子！"));
		return;
	}

	if(m_arrBoard[nHm][nHn] == NONE)
	{
		m_pGraphicsScene->addEllipse(nHx*40, nHy*40, 35, 35, QPen(Qt::NoPen), QColor(Qt::black));
		m_arrBoard[nHm][nHn] = BLACK;
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("无法落子！"));
		return;
	}
	
	//判断人类是否胜利
	if(m_pJugdeWinner->IsWon(BLACK, m_arrBoard))
	{
		QMessageBox::information(this, QString::fromLocal8Bit("Winner"),QString::fromLocal8Bit("你战胜了计算机！"));
		CleanBoard();
		ui.startButton->setText(QString::fromLocal8Bit("开始"));
		ui.startButton->setChecked(false);
		m_bStarted = false;
		return;
	}

	//机器落子
	ComputerMove* pComputerMove = new ComputerMove();
	//pComputerMove->Computer_1(m_arrBoard);
	pComputerMove->MaxMinSearch(m_arrBoard, m_nDeep);
	int nCm = pComputerMove->X();
	int nCn = pComputerMove->Y();
	delete pComputerMove;

	m_arrBoard[nCm][nCn] = WHITE;

	m_pGraphicsScene->addEllipse((nCm-8)*40, (nCn-8)*40, 35, 35, QPen(Qt::NoPen), QColor(Qt::white));
	
	//判断计算机是否胜利
	if(m_pJugdeWinner->IsWon(WHITE, m_arrBoard))
	{
		QMessageBox::information(this, QString::fromLocal8Bit("Winner"),QString::fromLocal8Bit("计算机取得胜利！"));
		CleanBoard();
		ui.startButton->setText(QString::fromLocal8Bit("开始"));
		ui.startButton->setChecked(false);
		m_bStarted = false;
		return;
	}
}

void Backgammon::CleanBoard()
{
	//删除棋子和线
	QList<QGraphicsItem*> list = m_pGraphicsScene->items();
	while(!list.isEmpty())
	{
		m_pGraphicsScene->removeItem(list[0]);
		list.removeFirst();
	}

	DrawBoard();  //划线

	//初始化数组
	for(int i = 0; i< 15; ++i)
	{
		for(int j = 0; j< 15; ++j)
		{
			m_arrBoard[i][j] = NONE;
		}
	}
}
