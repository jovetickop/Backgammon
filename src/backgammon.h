#ifndef BACKGAMMON_H
#define BACKGAMMON_H

#include <QtWidgets/QMainWindow>
#include <QString>
#include <QVector>
#include <QSettings>

#include "playerstatsstore.h"
#include "ui_backgammon.h"
#include "types.h"
#include "domain/services/win_detector.h"
#include "sgf_serializer.h"
#include "domain/services/board_evaluator.h"
#include "domain/services/ai_engine.h"

class QMouseEvent;
class QResizeEvent;
class QShowEvent;
class QGraphicsScene;
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;
class QGraphicsRectItem;
class WinRateChart;
class QPushButton;
class AiWorker;
class QThread;
class SoundManager;
class AchievementManager;

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
	void closeEvent(QCloseEvent *event) override;

	int CurrentDifficulty() const;
	void SetDifficulty(int depth);

public slots:
	void slotStartBtnClicked();
	void slotStarterChanged(int index);
	void slotDifficultyChanged(int index);
	void slotDifficultyTextChanged(const QString &text);
	void slotHistoryBtnClicked();
	void slotUndoBtnClicked();
	void slotThinkToggleClicked();
	// 接收「提示」按钮点击事件，切换 Top3 提示标记的显示/隐藏
	void slotHintBtnClicked(bool checked);
	// 接收 AiWorker 计算完成的落点
	void slotAiMoveReady(int row, int col);
	// 切换音效开关
	void slotSoundToggleClicked(bool checked);
	// 导出当前对局为 SGF 文件
	void slotExportSgfClicked();
	// 从 SGF 文件导入并回放棋谱
	void slotImportSgfClicked();

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
	// 计算玩家视角 Top3 推荐落点并绘制提示标记。
	void ComputeAndShowHint();
	// 清除棋盘上的 Top3 提示标记。
	void ClearHintOverlay();

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
	bool m_bPvPMode;  // 鍙屼汉瀵规垬妯″紡
	game_core::WinDetector m_winDetector;
	game_core::BoardEvaluator m_boardEvaluator;
	game_core::AIEngine m_aiEngine;
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

	// 「提示」按钮：点击后显示玩家视角 Top3 推荐落点。
	QPushButton *m_pHintBtn;
	// Top3 提示标记层的图形项（用于清除）。
	QVector<QGraphicsItem *> m_hintItems;
	// 是否正在显示提示标记。
	bool m_bHintVisible;

	// AI 异步计算线程和 Worker。
	QThread *m_pAiThread;
	AiWorker *m_pAiWorker;
	// 是否正在等待 AI 计算完成。
	bool m_bAiThinking;

	// 悔棋计数：当前局已悔棋次数 / 每局最大悔棋次数。
	int m_nUndoCount;
	int m_nMaxUndoCount;

	// 音效管理器及音效开关按钮
	SoundManager *m_pSoundManager;
	QPushButton *m_pSoundToggleBtn;
};

#endif // BACKGAMMON_H
