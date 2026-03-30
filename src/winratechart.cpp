#include "winratechart.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace
{
	// 图表边距：留出足够空间给Y轴刻度、图例、底部标签。
	const int kPaddingLeft = 52;
	const int kPaddingRight = 24;
	const int kPaddingTop = 64;
	const int kPaddingBottom = 44;
	const QColor kPlayerColor(44, 54, 74);
	const QColor kAiColor(209, 132, 47);

	QPainterPath BuildSmoothPath(const QVector<QPointF> &points)
	{
		QPainterPath path;
		if (points.isEmpty())
			return path;

		path.moveTo(points[0]);
		for (int i = 1; i < points.size(); ++i)
		{
			const QPointF p0 = points[i - 1];
			const QPointF p1 = points[i];
			const qreal midX = (p0.x() + p1.x()) * 0.5;
			path.cubicTo(QPointF(midX, p0.y()), QPointF(midX, p1.y()), p1);
		}
		return path;
	}
}

WinRateChart::WinRateChart(QWidget *parent)
	: QWidget(parent)
{
	setMinimumHeight(300);
	setAttribute(Qt::WA_TranslucentBackground);
}

void WinRateChart::SetSeries(const QVector<double> &playerRates, const QVector<double> &aiRates)
{
	m_playerRates = playerRates;
	m_aiRates = aiRates;
	update();
}

void WinRateChart::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(rect(), QColor(255, 255, 255, 0));

	QRectF chartRect(kPaddingLeft, kPaddingTop,
		width() - kPaddingLeft - kPaddingRight,
		height() - kPaddingTop - kPaddingBottom);

	if (chartRect.width() <= 0 || chartRect.height() <= 0)
		return;

	QPen axisPen(QColor(116, 128, 145, 95));
	axisPen.setWidth(1);
	painter.setPen(axisPen);

	for (int i = 0; i <= 4; ++i)
	{
		const double ratio = i / 4.0;
		const qreal y = chartRect.bottom() - ratio * chartRect.height();
		painter.drawLine(chartRect.left() + 4, y, chartRect.right(), y);
		// Y轴刻度标签：增大字体到13px，确保数字清晰可读。
		QFont axisFont = painter.font();
		axisFont.setPixelSize(13);
		painter.setFont(axisFont);
		painter.setPen(QColor(96, 108, 124, 180));
		painter.drawText(QRectF(0, y - 12, kPaddingLeft - 8, 24),
			Qt::AlignRight | Qt::AlignVCenter,
			QString::number(static_cast<int>(ratio * 100)) + "%");
		painter.setPen(axisPen);
	}

	painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
	painter.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

	// 图例：增大字体和间距，确保文字显示完整。
	QFont legendFont = painter.font();
	legendFont.setPixelSize(14);
	legendFont.setBold(true);
	painter.setFont(legendFont);

	QPen legendPlayerPen(kPlayerColor);
	legendPlayerPen.setWidth(3);
	painter.setPen(legendPlayerPen);
	painter.drawLine(QPointF(chartRect.left(), 22), QPointF(chartRect.left() + 24, 22));
	painter.setPen(QColor(74, 86, 105));
	painter.drawText(QRectF(chartRect.left() + 32, 8, 90, 28), Qt::AlignVCenter, QString::fromUtf8(u8"\u6211"));

	QPen legendAiPen(kAiColor);
	legendAiPen.setWidth(3);
	painter.setPen(legendAiPen);
	painter.drawLine(QPointF(chartRect.left() + 110, 22), QPointF(chartRect.left() + 134, 22));
	painter.setPen(QColor(74, 86, 105));
	painter.drawText(QRectF(chartRect.left() + 142, 8, 90, 28), Qt::AlignVCenter, QString::fromUtf8("AI"));

	if (m_playerRates.isEmpty() || m_aiRates.isEmpty())
	{
		QFont hintFont = painter.font();
		hintFont.setPixelSize(14);
		painter.setFont(hintFont);
		painter.setPen(QColor(98, 111, 128, 200));
		painter.drawText(chartRect, Qt::AlignCenter, QString::fromUtf8(u8"\u5f00\u5c40\u540e\u663e\u793a\u8fd9\u4e00\u5c40\u7684\u80dc\u7387\u53d8\u5316"));
		return;
	}

	const int pointCount = qMin(m_playerRates.size(), m_aiRates.size());
	if (pointCount <= 0)
		return;

	const auto valueToPoint = [&](int index, double value) {
		const qreal x = pointCount == 1
			? chartRect.center().x()
			: chartRect.left() + (chartRect.width() * index) / (pointCount - 1);
		const qreal y = chartRect.bottom() - (value / 100.0) * chartRect.height();
		return QPointF(x, y);
	};

	QVector<QPointF> playerPoints;
	QVector<QPointF> aiPoints;
	for (int i = 0; i < pointCount; ++i)
	{
		playerPoints.push_back(valueToPoint(i, m_playerRates[i]));
		aiPoints.push_back(valueToPoint(i, m_aiRates[i]));
	}

	const QPainterPath playerPath = BuildSmoothPath(playerPoints);
	const QPainterPath aiPath = BuildSmoothPath(aiPoints);

	QPen playerPen(kPlayerColor);
	playerPen.setWidth(4);
	painter.setPen(playerPen);
	painter.drawPath(playerPath);

	QPen aiPen(kAiColor);
	aiPen.setWidth(4);
	painter.setPen(aiPen);
	painter.drawPath(aiPath);

	for (int i = 0; i < pointCount; ++i)
	{
		painter.setPen(QPen(kPlayerColor, 2));
		painter.setBrush(QColor(255, 255, 255, 245));
		painter.drawEllipse(playerPoints[i], 5, 5);

		painter.setPen(QPen(kAiColor, 2));
		painter.drawEllipse(aiPoints[i], 5, 5);
	}

	const QPointF latestPlayer = playerPoints.last();
	const QPointF latestAi = aiPoints.last();

	// 右上角最新胜率标签：加宽至130px，字体14px，确保"我 100%"和"AI 100%"完整显示。
	const int labelW = 130;
	const int labelH = 32;
	const int labelX = chartRect.right() - labelW - 8;
	QFont labelFont = painter.font();
	labelFont.setPixelSize(14);
	labelFont.setBold(true);
	painter.setFont(labelFont);

	painter.setPen(QPen(kPlayerColor, 2));
	painter.setBrush(QColor(255, 255, 255, 240));
	painter.drawRoundedRect(QRectF(labelX, chartRect.top() + 10, labelW, labelH), 10, 10);
	painter.setPen(kPlayerColor);
	painter.drawText(QRectF(labelX, chartRect.top() + 10, labelW, labelH),
		Qt::AlignCenter, QString::fromUtf8(u8"\u6211 %1%").arg(qRound(m_playerRates.last())));

	painter.setPen(QPen(kAiColor, 2));
	painter.setBrush(QColor(255, 255, 255, 240));
	painter.drawRoundedRect(QRectF(labelX, chartRect.top() + 10 + labelH + 6, labelW, labelH), 10, 10);
	painter.setPen(kAiColor);
	painter.drawText(QRectF(labelX, chartRect.top() + 10 + labelH + 6, labelW, labelH),
		Qt::AlignCenter, QString::fromUtf8("AI %1%").arg(qRound(m_aiRates.last())));

	painter.setPen(QPen(kPlayerColor, 2));
	painter.setBrush(QColor(255, 255, 255, 250));
	painter.drawEllipse(latestPlayer, 5, 5);
	painter.setPen(QPen(kAiColor, 2));
	painter.drawEllipse(latestAi, 5, 5);

	painter.setPen(QColor(96, 108, 124, 180));
	QFont bottomFont = painter.font();
	bottomFont.setPixelSize(13);
	painter.setFont(bottomFont);
	painter.drawText(QRectF(chartRect.left(), chartRect.bottom() + 12, chartRect.width(), 24),
		Qt::AlignCenter,
		QString::fromUtf8(u8"\u5f53\u524d\u5c40\u6b65\u6570"));
}
