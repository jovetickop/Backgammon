#include "winratechart.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

namespace
{
	const int kPaddingLeft = 42;
	const int kPaddingRight = 18;
	const int kPaddingTop = 54;
	const int kPaddingBottom = 34;
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
		painter.setPen(QColor(96, 108, 124, 180));
		painter.drawText(QRectF(0, y - 10, kPaddingLeft - 8, 20),
			Qt::AlignRight | Qt::AlignVCenter,
			QString::number(static_cast<int>(ratio * 100)) + "%");
		painter.setPen(axisPen);
	}

	painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());
	painter.drawLine(chartRect.bottomLeft(), chartRect.topLeft());

	painter.setPen(Qt::NoPen);
	painter.setBrush(kPlayerColor);
	painter.drawRoundedRect(QRectF(chartRect.left(), 16, 20, 10), 4, 4);
	painter.setPen(QColor(74, 86, 105));
	painter.drawText(QRectF(chartRect.left() + 28, 8, 80, 24), Qt::AlignVCenter, QString::fromUtf8(u8"\u6211"));

	painter.setPen(Qt::NoPen);
	painter.setBrush(kAiColor);
	painter.drawRoundedRect(QRectF(chartRect.left() + 90, 16, 20, 10), 4, 4);
	painter.setPen(QColor(74, 86, 105));
	painter.drawText(QRectF(chartRect.left() + 118, 8, 80, 24), Qt::AlignVCenter, QString::fromUtf8("AI"));

	if (m_playerRates.isEmpty() || m_aiRates.isEmpty())
	{
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
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(255, 255, 255, 210));
		painter.drawEllipse(playerPoints[i], 6, 6);
		painter.drawEllipse(aiPoints[i], 6, 6);

		painter.setBrush(kPlayerColor);
		painter.drawEllipse(playerPoints[i], 3.5, 3.5);
		painter.setBrush(kAiColor);
		painter.drawEllipse(aiPoints[i], 3.5, 3.5);
	}

	const QPointF latestPlayer = playerPoints.last();
	const QPointF latestAi = aiPoints.last();

	painter.setPen(Qt::NoPen);
	painter.setBrush(kPlayerColor);
	painter.drawRoundedRect(QRectF(chartRect.right() - 116, chartRect.top() + 10, 104, 28), 10, 10);
	painter.setPen(Qt::white);
	painter.drawText(QRectF(chartRect.right() - 116, chartRect.top() + 10, 104, 28),
		Qt::AlignCenter, QString::fromUtf8(u8"\u6211 %1%").arg(qRound(m_playerRates.last())));

	painter.setPen(Qt::NoPen);
	painter.setBrush(kAiColor);
	painter.drawRoundedRect(QRectF(chartRect.right() - 116, chartRect.top() + 44, 104, 28), 10, 10);
	painter.setPen(Qt::white);
	painter.drawText(QRectF(chartRect.right() - 116, chartRect.top() + 44, 104, 28),
		Qt::AlignCenter, QString::fromUtf8("AI %1%").arg(qRound(m_aiRates.last())));

	painter.setPen(Qt::NoPen);
	painter.setBrush(kPlayerColor);
	painter.drawEllipse(latestPlayer, 5, 5);
	painter.setBrush(kAiColor);
	painter.drawEllipse(latestAi, 5, 5);

	painter.setPen(QColor(96, 108, 124, 180));
	painter.drawText(QRectF(chartRect.left(), chartRect.bottom() + 10, chartRect.width(), 20),
		Qt::AlignCenter,
		QString::fromUtf8(u8"\u5f53\u524d\u5c40\u6b65\u6570"));
}
