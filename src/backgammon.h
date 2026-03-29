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
class judgeWinner;
class WinRateChart;
class Evaluation;

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

private:
	void RecordGameResult(ePiece winner);
	void PersistPlayerRecord();
	void ResetWinRateEstimate();
	void UpdateStatsPanel();
	void UpdateWinRateEstimate();
	void UpdateBoardView();
	void SetLastAiPiece(QGraphicsEllipseItem *piece);
	void AppendMove(int row, int col, ePiece piece);
	void PlaceAiMove(int row, int col);
	void PlacePlayerMove(int row, int col);
	void PlaceAiOpeningMove();
	void FinishRoundCleanup();
	QString CurrentStarterPreference() const;

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
};

#endif // BACKGAMMON_H
