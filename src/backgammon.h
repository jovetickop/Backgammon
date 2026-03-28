#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include <QtWidgets/QMainWindow>
#include <QString>
#include <QVector>
#include "ui_backgammon.h"
#include "types.h"

class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QGraphicsScene;
class QGraphicsEllipseItem;
class judgeWinner;
class WinRateChart;
class Evaluation;
class PlayerStatsStore;
struct PlayerRecord;

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

public slots:
	void slotStartBtnClicked();

private:
	void RecordGameResult(ePiece winner);
	void PersistPlayerRecord();
	void ResetWinRateEstimate();
	void UpdateStatsPanel();
	void UpdateWinRateEstimate();
	void UpdateBoardView();
	void SetLastAiPiece(QGraphicsEllipseItem *piece);

	Ui::BackgammonClass ui;
	ePiece m_arrBoard[15][15];
	QGraphicsScene *m_pGraphicsScene;
	QGraphicsEllipseItem *m_pLastAiPiece;
	WinRateChart *m_pWinRateChart;
	PlayerStatsStore *m_pStatsStore;
	QString m_sCurrentUser;
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
	int m_nDeep;
};

#endif // BACKGAMMON_H
