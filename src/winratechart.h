#ifndef WINRATECHART_H
#define WINRATECHART_H

#include <QWidget>
#include <QVector>

class WinRateChart : public QWidget
{
	Q_OBJECT

public:
	explicit WinRateChart(QWidget *parent = 0);

	void SetSeries(const QVector<double> &playerRates, const QVector<double> &aiRates);

protected:
	void paintEvent(QPaintEvent *event);

private:
	QVector<double> m_playerRates;
	QVector<double> m_aiRates;
};

#endif // WINRATECHART_H
