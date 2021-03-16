#ifndef WAVEFORM_H
#define WAVEFORM_H

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

class Waveform : QObject
{
	Q_OBJECT

public:
	static const int DEFAULT_SAMPLE_COUNT = 2000;

	Waveform(QString title = "", int sampleCount = DEFAULT_SAMPLE_COUNT, QObject* parent = nullptr);
	QChartView* getChartView();
	QLineSeries* getSeries();
	QChart* getChart();
	QValueAxis* getAxisX();
	QValueAxis* getAxisY();
	int getSampleCount();

private:
	QChart* m_waveformChart;
	QLineSeries* m_waveformSeries;
	QChartView* m_waveformChartView;
	QValueAxis* m_axisX;
	QValueAxis* m_axisY;
	int m_sampleCount;
};

#endif // WAVEFORM_H