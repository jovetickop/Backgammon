#ifndef REPLAYDIALOG_H
#define REPLAYDIALOG_H

#include <QDialog>

#include "playerstatsstore.h"

class QGraphicsScene;
class QGraphicsView;
class QPushButton;
class QLabel;
class QComboBox;
class QTimer;

// 棋谱回放对话框：逐步展示历史对局的落子序列。
// 支持上一步/下一步/自动播放（慢/中/快三档速度）。
// 回放期间禁止落子操作，仅允许控制播放进度。
class ReplayDialog : public QDialog
{
	Q_OBJECT

public:
	// 构造回放对话框。
	// @param record   要回放的对局记录（含完整落子序列）
	// @param parent   父窗口
	explicit ReplayDialog(const GameRecord &record, QWidget *parent = nullptr);
	~ReplayDialog() override;

private slots:
	// 后退一步。
	void onPrevClicked();
	// 前进一步。
	void onNextClicked();
	// 切换自动播放状态（开始/暂停）。
	void onAutoPlayToggled();
	// 自动播放计时器触发，等同于点击下一步。
	void onTimerTick();
	// 速度档位变化时更新计时器间隔。
	void onSpeedChanged(int index);

private:
	// 清空并重绘棋盘到当前步骤状态。
	void RenderCurrentStep();
	// 绘制棋盘背景、网格线和星位点。
	void DrawBoard();
	// 根据当前步数同步按钮启用状态和步数标签。
	void UpdateControls();
	// 棋盘逻辑坐标 -> 场景坐标（以棋盘中心为原点）。
	int BoardToScene(int index) const;

	GameRecord m_record;	// 回放的对局记录
	int m_currentStep;		// 当前已显示步数（0=空棋盘，N=前N步）

	QGraphicsScene *m_pScene;
	QGraphicsView  *m_pView;
	QPushButton    *m_pPrevBtn;
	QPushButton    *m_pNextBtn;
	QPushButton    *m_pAutoBtn;
	QComboBox      *m_pSpeedCombo;
	QLabel         *m_pStepLabel;
	QTimer         *m_pTimer;

	// 自动播放速度档位对应的毫秒间隔。
	static constexpr int kSpeedSlow   = 1500;
	static constexpr int kSpeedMedium = 800;
	static constexpr int kSpeedFast   = 300;

	// 棋盘绘制常量（略小于主窗口以适应对话框尺寸）。
	static constexpr int kBoardSize   = 15;
	static constexpr int kBoardCenter = 7;
	static constexpr int kGridSize    = 40;
	static constexpr int kPieceSize   = 34;
	static constexpr int kPieceRadius = kPieceSize / 2;
	static constexpr int kLineMin     = -kBoardCenter * kGridSize;
	static constexpr int kLineMax     =  kBoardCenter * kGridSize;
};

#endif // REPLAYDIALOG_H
