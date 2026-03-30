#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QVector>

#include "playerstatsstore.h"

class QListWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QDateEdit;

// 历史对局对话框：展示玩家历史对局列表，支持多选删除和棋谱回放。
// 删除操作通过 deletedIndices 信号传出，由调用方（Backgammon）实际执行数据删除。
// 回放操作通过 replayRequested 信号传出，携带被选中对局的 games 数组索引。
class HistoryDialog : public QDialog
{
	Q_OBJECT

public:
	// 构造历史对局对话框。
	// @param userName   玩家显示名称
	// @param record     玩家完整记录（含所有历史对局）
	// @param parent     父窗口
	explicit HistoryDialog(const QString &userName, const PlayerRecord &record, QWidget *parent = 0);

signals:
	// 当用户点击"删除选中"按钮时发出，携带被选中的对局在 games 数组中的索引。
	// 索引基于原始 record.games 的下标（0-based），调用方需据此删除对应记录。
	void deletedIndices(const QVector<int> &indices);

	// 当用户点击"回放"按钮时发出，携带被选中对局在 games 数组中的索引。
	void replayRequested(int gameIndex);

private slots:
	// 处理删除选中按钮点击：弹出确认框后发出 deletedIndices 信号。
	void onDeleteClicked();
	// 处理回放按钮点击：发出 replayRequested 信号。
	void onReplayClicked();
	// 处理列表选择变化：更新按钮的可用状态。
	void onSelectionChanged();
	// 筛选条件变化时重新过滤列表。
	void onFilterChanged();

private:
	// 根据当前筛选条件重新填充列表。
	void applyFilter();

	QListWidget *m_pHistoryList;
	QPushButton *m_pDeleteButton;
	QPushButton *m_pReplayButton;
	// 筛选控件
	QLineEdit  *m_pSearchEdit;   // 关键词搜索框
	QComboBox  *m_pResultFilter; // 胜负筛选（全部/胜利/失利）
	QDateEdit  *m_pDateFrom;     // 日期范围起始
	QDateEdit  *m_pDateTo;       // 日期范围结束

	const PlayerRecord &m_record; // 完整玩家记录（用于重建筛选后列表）
	QVector<int> m_gameIndices; // 列表每一项对应的原始 games 数组索引（逆序排列）
};

#endif // HISTORYDIALOG_H
