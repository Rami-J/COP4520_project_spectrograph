#ifndef SPECTROGRAPH_H
#define SPECTROGRAPH_H

#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QXYSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class Spectrograph : QObject
{
	Q_OBJECT

public:
	Spectrograph(QString title = "", QObject* parent = nullptr);
	QChartView* getChartView();
	QLineSeries* getSeries();
	QChart* getChart();
	QValueAxis* getAxisX();
	QValueAxis* getAxisY();

private:
	QChart* m_spectrumChart;
	QLineSeries* m_spectrumSeries;
	QChartView* m_spectrumChartView;
	QValueAxis* m_axisX;
	QValueAxis* m_axisY;
};

#endif // SPECTROGRAPH_H
