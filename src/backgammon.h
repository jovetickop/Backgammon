#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include <QtWidgets/QMainWindow>
#include <QString>
#include <QVector>

#include "playerstatsstore.h"
#include "ui_backgammon.h"
#include "types.h"

class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QGraphicsScene;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;
class QGraphicsRectItem;
class judgeWinner;
class WinRateChart;
class Evaluation;
class QPushButton;
class ComputerMove;

class Backgammon : public QMainWindow
{
	Q_OBJECT

public:
	Backgammon(PlayerStatsStore *statsStore, const PlayerRecord &playerRecord, QWidget *parent = 0);
	~Backgammon();

	void DrawBoard();
	bool IsBoardClean();
	void mousePressEvent(QMouseEvent * event);
	void showEvent(QShowEvent *event);
	void resizeEvent(QResizeEvent *event);
	void CleanBoard();

	int CurrentDifficulty() const;
	void SetDifficulty(int depth);

public slots:
	void slotStartBtnClicked();
	void slotStarterChanged(int index);
	void slotDifficultyChanged(int index);
	void slotDifficultyTextChanged(const QString &text);
	void slotHistoryBtnClicked();
	void slotThinkToggleClicked();

private:
	void RecordGameResult(ePiece winner);
	void PersistPlayerRecord();
	void ResetWinRateEstimate();
	void UpdateStatsPanel();
	// 更新胜率估算：nextPiece 表示即将落子的棋子（即当前轮到谁走）。
	void UpdateWinRateEstimate(ePiece nextPiece);
	void UpdateBoardView();
	void SetLastAiPiece(QGraphicsEllipseItem *piece);
	void AppendMove(int row, int col, ePiece piece);
	void PlaceAiMove(int row, int col);
	void PlacePlayerMove(int row, int col);
	void PlaceAiOpeningMove();
	void FinishRoundCleanup();
	QString CurrentStarterPreference() const;

	// 计算 Top10 候选点并绘制在棋盘上，显示每个位置的 AI 评估胜率。
	void ComputeAndShowTop10();
	// 清除棋盘上的 Top10 标记（半透明圆形、排名文字、胜率文字）。
	void ClearTop10Overlay();

	Ui::BackgammonClass ui;
	ePiece m_arrBoard[15][15];
	QGraphicsScene *m_pGraphicsScene;
	QGraphicsEllipseItem *m_pLastAiPiece;
	WinRateChart *m_pWinRateChart;
	PlayerStatsStore *m_pStatsStore;
	PlayerRecord m_playerRecord;
	QString m_sCurrentUser;
	bool m_bStarted;
	bool m_bPlayerStarts;
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
	QVector<MoveRecord> m_currentGameMoves;
	int m_nDeep;
	int m_nPreferredDeep;

	// "AI 思考"切换按钮：按下后在棋盘上显示 AI 评估的 Top10 候选点及胜率。
	QPushButton *m_pThinkToggleBtn;
	// Top10 标记层的图形项（用于清除/重绘）。
	QVector<QGraphicsItem *> m_top10Items;
	// 是否正在显示 Top10 标记。
	bool m_bShowTop10;
};

#endif // BACKGAMMON_H
