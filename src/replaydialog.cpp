#include "replaydialog.h"

#include <QComboBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>
#include <QPen>
#include <QPushButton>
#include <QRadialGradient>
#include <QTimer>
#include <QVBoxLayout>

// -----------------------------------------------------------------------
// 构造 / 析构
// -----------------------------------------------------------------------

ReplayDialog::ReplayDialog(const GameRecord &record, QWidget *parent)
	: QDialog(parent)
	, m_record(record)
	, m_currentStep(0)
	, m_pScene(new QGraphicsScene(this))
	, m_pView(new QGraphicsView(m_pScene, this))
	, m_pPrevBtn(new QPushButton(QString::fromUtf8(u8"◀ 上一步"), this))
	, m_pNextBtn(new QPushButton(QString::fromUtf8(u8"下一步 ▶"), this))
	, m_pAutoBtn(new QPushButton(QString::fromUtf8(u8"▶ 自动播放"), this))
	, m_pSpeedCombo(new QComboBox(this))
	, m_pStepLabel(new QLabel(this))
	, m_pTimer(new QTimer(this))
{
	setModal(true);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowTitle(QString::fromUtf8(u8"棋谱回放"));
	setMinimumSize(700, 720);

	// 速度选项
	m_pSpeedCombo->addItem(QString::fromUtf8(u8"慢速"),   kSpeedSlow);
	m_pSpeedCombo->addItem(QString::fromUtf8(u8"中速"),   kSpeedMedium);
	m_pSpeedCombo->addItem(QString::fromUtf8(u8"快速"),   kSpeedFast);
	m_pSpeedCombo->setCurrentIndex(1); // 默认中速

	// 棋盘视图配置
	m_pScene->setSceneRect(
		kLineMin - kGridSize, kLineMin - kGridSize,
		(kLineMax - kLineMin) + 2 * kGridSize,
		(kLineMax - kLineMin) + 2 * kGridSize);
	QLinearGradient boardBg(
		kLineMin, kLineMin,
		kLineMax, kLineMax);
	boardBg.setColorAt(0.0, QColor(245, 222, 179));
	boardBg.setColorAt(0.5, QColor(238, 213, 168));
	boardBg.setColorAt(1.0, QColor(225, 200, 155));
	m_pView->setBackgroundBrush(QBrush(boardBg));
	m_pView->setRenderHint(QPainter::Antialiasing);
	m_pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 步数标签
	m_pStepLabel->setAlignment(Qt::AlignCenter);
	m_pStepLabel->setStyleSheet("font-size: 15px; color: rgb(55,70,90);");

	// 控制按钮行
	QHBoxLayout *btnLayout = new QHBoxLayout;
	btnLayout->setSpacing(10);
	btnLayout->addWidget(m_pPrevBtn);
	btnLayout->addWidget(m_pNextBtn);
	btnLayout->addStretch();
	btnLayout->addWidget(new QLabel(QString::fromUtf8(u8"速度:"), this));
	btnLayout->addWidget(m_pSpeedCombo);
	btnLayout->addStretch();
	btnLayout->addWidget(m_pAutoBtn);

	// 主布局
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(16, 16, 16, 16);
	mainLayout->setSpacing(10);
	mainLayout->addWidget(m_pView, 1);
	mainLayout->addWidget(m_pStepLabel);
	mainLayout->addLayout(btnLayout);

	// 信号连接
	connect(m_pPrevBtn,    &QPushButton::clicked,       this, &ReplayDialog::onPrevClicked);
	connect(m_pNextBtn,    &QPushButton::clicked,       this, &ReplayDialog::onNextClicked);
	connect(m_pAutoBtn,    &QPushButton::clicked,       this, &ReplayDialog::onAutoPlayToggled);
	connect(m_pSpeedCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &ReplayDialog::onSpeedChanged);
	connect(m_pTimer,      &QTimer::timeout,            this, &ReplayDialog::onTimerTick);

	// 初始渲染
	DrawBoard();
	UpdateControls();
}

ReplayDialog::~ReplayDialog()
{
	// m_pTimer 是子对象，Qt 父子关系自动释放。
}

// -----------------------------------------------------------------------
// 槽函数
// -----------------------------------------------------------------------

void ReplayDialog::onPrevClicked()
{
	if (m_currentStep > 0)
	{
		--m_currentStep;
		RenderCurrentStep();
		UpdateControls();
	}
}

void ReplayDialog::onNextClicked()
{
	const int total = m_record.moves.size();
	if (m_currentStep < total)
	{
		++m_currentStep;
		RenderCurrentStep();
		UpdateControls();
		// 到达末尾时停止自动播放。
		if (m_currentStep >= total && m_pTimer->isActive())
		{
			m_pTimer->stop();
			m_pAutoBtn->setText(QString::fromUtf8(u8"▶ 自动播放"));
		}
	}
}

void ReplayDialog::onAutoPlayToggled()
{
	if (m_pTimer->isActive())
	{
		// 暂停
		m_pTimer->stop();
		m_pAutoBtn->setText(QString::fromUtf8(u8"▶ 自动播放"));
	}
	else
	{
		// 若已在末尾则从头开始
		if (m_currentStep >= m_record.moves.size())
		{
			m_currentStep = 0;
			RenderCurrentStep();
			UpdateControls();
		}
		const int interval = m_pSpeedCombo->currentData().toInt();
		m_pTimer->start(interval);
		m_pAutoBtn->setText(QString::fromUtf8(u8"⏸ 暂停"));
	}
}

void ReplayDialog::onTimerTick()
{
	onNextClicked();
}

void ReplayDialog::onSpeedChanged(int /*index*/)
{
	// 若正在自动播放，实时更新间隔。
	if (m_pTimer->isActive())
	{
		const int interval = m_pSpeedCombo->currentData().toInt();
		m_pTimer->setInterval(interval);
	}
}

// -----------------------------------------------------------------------
// 渲染辅助
// -----------------------------------------------------------------------

void ReplayDialog::RenderCurrentStep()
{
	// 清除场景后重绘棋盘和前 m_currentStep 步棋子。
	m_pScene->clear();
	DrawBoard();

	for (int i = 0; i < m_currentStep && i < m_record.moves.size(); ++i)
	{
		const MoveRecord &mv = m_record.moves[i];
		const int x = BoardToScene(mv.row) - kPieceRadius;
		const int y = BoardToScene(mv.col) - kPieceRadius;
		const qreal cx = x + kPieceRadius - 5;
		const qreal cy = y + kPieceRadius - 5;

		if (mv.piece == WHITE)
		{
			// 白子：径向渐变模拟球体高光。
			QRadialGradient grad(cx, cy, kPieceRadius);
			grad.setColorAt(0.0,  QColor(255, 255, 255));
			grad.setColorAt(0.6,  QColor(235, 235, 235));
			grad.setColorAt(0.85, QColor(200, 200, 200));
			grad.setColorAt(1.0,  QColor(160, 160, 160));
			m_pScene->addEllipse(
				x, y, kPieceSize, kPieceSize,
				QPen(QColor(140, 140, 140, 120), 1),
				QBrush(grad));
		}
		else if (mv.piece == BLACK)
		{
			// 黑子：径向渐变模拟深色球体。
			QRadialGradient grad(cx, cy, kPieceRadius);
			grad.setColorAt(0.0,  QColor(90, 90, 90));
			grad.setColorAt(0.5,  QColor(50, 50, 50));
			grad.setColorAt(0.85, QColor(25, 25, 25));
			grad.setColorAt(1.0,  QColor(10, 10, 10));
			m_pScene->addEllipse(
				x, y, kPieceSize, kPieceSize,
				QPen(QColor(5, 5, 5, 80), 1),
				QBrush(grad));
		}

		// 最后一步高亮：红色外框标记最新落点。
		if (i == m_currentStep - 1)
		{
			m_pScene->addEllipse(
				x - 2, y - 2,
				kPieceSize + 4, kPieceSize + 4,
				QPen(QColor(220, 60, 60, 200), 2),
				QBrush(Qt::NoBrush));
		}
	}
}

void ReplayDialog::DrawBoard()
{
	// 外框。
	QPen borderPen(QColor(60, 40, 20, 200));
	borderPen.setWidth(2);
	const int borderOffset = 6;
	m_pScene->addRect(
		kLineMin - borderOffset, kLineMin - borderOffset,
		(kLineMax - kLineMin) + 2 * borderOffset,
		(kLineMax - kLineMin) + 2 * borderOffset,
		borderPen, QBrush(QColor(140, 100, 55, 60)));

	// 网格线。
	QPen gridLine(QColor(60, 40, 18, 160));
	gridLine.setWidth(1);
	for (int i = 0; i < kBoardSize; ++i)
	{
		const int coord = BoardToScene(i);
		m_pScene->addLine(kLineMin, coord, kLineMax, coord, gridLine);
		m_pScene->addLine(coord, kLineMin, coord, kLineMax, gridLine);
	}

	// 星位点。
	const int starPoints[][2] = {
		{7, 7}, {3, 3}, {3, 11}, {11, 3}, {11, 11}
	};
	const int starRadius = 4;
	for (int i = 0; i < 5; ++i)
	{
		const int cx = BoardToScene(starPoints[i][0]);
		const int cy = BoardToScene(starPoints[i][1]);
		QRadialGradient starGrad(cx - 1, cy - 1, starRadius);
		starGrad.setColorAt(0.0, QColor(80, 55, 25));
		starGrad.setColorAt(1.0, QColor(50, 35, 15));
		m_pScene->addEllipse(
			cx - starRadius, cy - starRadius,
			starRadius * 2, starRadius * 2,
			QPen(Qt::NoPen), QBrush(starGrad));
	}
}

void ReplayDialog::UpdateControls()
{
	const int total = m_record.moves.size();
	m_pPrevBtn->setEnabled(m_currentStep > 0);
	m_pNextBtn->setEnabled(m_currentStep < total);
	m_pAutoBtn->setEnabled(total > 0);
	m_pStepLabel->setText(
		QString::fromUtf8(u8"第 %1 步 / 共 %2 步")
			.arg(m_currentStep)
			.arg(total));
}

int ReplayDialog::BoardToScene(int index) const
{
	return (index - kBoardCenter) * kGridSize;
}
