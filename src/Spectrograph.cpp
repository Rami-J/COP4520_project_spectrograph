#include "Spectrograph.h"

Spectrograph::Spectrograph(QString title, QObject* parent) 
	: QObject(parent)
	, m_spectrumChart(new QChart)
	, m_spectrumSeries(new QLineSeries)
    , m_spectrumChartView(new QChartView(m_spectrumChart))
    , m_axisX(new QValueAxis)
    , m_axisY(new QValueAxis)
{
    m_spectrumChartView->resize(800, 600);
    m_spectrumChartView->setMinimumSize(380, 300);
    m_spectrumChart->addSeries(m_spectrumSeries);
    m_axisX->setRange(0, 1000);
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Frequency (Hz)");
    m_axisY = new QValueAxis;
    m_axisY->setRange(-1, 1);
    m_axisY->setTitleText("Amplitude");
    m_spectrumChart->addAxis(m_axisX, Qt::AlignBottom);
    m_spectrumSeries->attachAxis(m_axisX);
    m_spectrumChart->addAxis(m_axisY, Qt::AlignLeft);
    m_spectrumSeries->attachAxis(m_axisY);
    m_spectrumChart->legend()->hide();
    m_spectrumChart->setTitle(title);
}

QChartView* Spectrograph::getChartView()
{
    return m_spectrumChartView;
}

QLineSeries* Spectrograph::getSeries()
{
    return m_spectrumSeries;
}

QChart* Spectrograph::getChart()
{
    return m_spectrumChart;
}

QValueAxis* Spectrograph::getAxisX()
{
    return m_axisX;
}

QValueAxis* Spectrograph::getAxisY()
{
    return m_axisY;
}