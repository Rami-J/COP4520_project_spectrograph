#ifndef SPECTROGRAPH_H
#define SPECTROGRAPH_H

#include "FTController.h"

#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QXYSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QBuffer>
#include <QAudioDecoder>

#define _USE_MATH_DEFINES

QT_CHARTS_BEGIN_NAMESPACE
class QLineSeries;
class QChart;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class Spectrograph : public QObject
{
	Q_OBJECT

public:
	Spectrograph(QString title = "", QObject* parent = nullptr);
	QChartView* getChartView();
	QLineSeries* getSeries();
	QChart* getChart();
	QValueAxis* getAxisX();
	QValueAxis* getAxisY();
	QBuffer* getDataBuffer();
	void cancelCalculation();

    void calculateSpectrum(const QAudioFormat format);

private slots:
    void plotSpectrumData(const QVector<QPointF> points, const double elapsedSeconds);

private:
	QChart* m_spectrumChart;
	QLineSeries* m_spectrumSeries;
	QChartView* m_spectrumChartView;
	QValueAxis* m_axisX;
	QValueAxis* m_axisY;
	FTController* m_FTController;
};

#endif // SPECTROGRAPH_H
