#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include <QtWidgets/QMainWindow>
#include <QVector>
#include "ui_backgammon.h"
#include "types.h"

class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QGraphicsScene;
class judgeWinner;
class WinRateChart;
class Evaluation;

// 主窗口：负责界面绘制、交互流程和人机对弈调度。
class Backgammon : public QMainWindow
{
	Q_OBJECT

public:
	Backgammon(QWidget *parent = 0);
	~Backgammon();

	// 绘制棋盘网格线。
	void DrawBoard();
	// 判断棋盘是否为空盘（仅剩网格线）。
	bool IsBoardClean();
	// 处理鼠标落子。
	void mousePressEvent(QMouseEvent * event);
	// 窗口显示与尺寸变化时同步刷新棋盘缩放。
	void showEvent(QShowEvent *event);
	void resizeEvent(QResizeEvent *event);
	// 清空棋盘并重置数据。
	void CleanBoard();

public slots:
	// 开始/清除按钮处理。
	void slotStartBtnClicked();

private:
	void RecordGameResult(ePiece winner);
	void ResetWinRateEstimate();
	void UpdateStatsPanel();
	void UpdateWinRateEstimate();
	void UpdateBoardView();

	Ui::BackgammonClass ui;
	ePiece m_arrBoard[15][15];
	QGraphicsScene *m_pGraphicsScene;
	WinRateChart *m_pWinRateChart;
	bool m_bStarted;
	judgeWinner* m_pJugdeWinner;
	Evaluation *m_pEvaluation;
	int m_nPlayerWins;
	int m_nAiWins;
	int m_nFinishedGames;
	int m_nMoveCount;
	double m_nPlayerWinRate;
	double m_nAiWinRate;
	QVector<double> m_playerRateHistory;
	QVector<double> m_aiRateHistory;

	// 搜索深度：数值越大棋力越高，耗时也越高。
	int m_nDeep;
};

#endif // BACKGAMMON_H
