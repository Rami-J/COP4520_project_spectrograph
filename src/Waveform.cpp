#include "Waveform.h"

Waveform::Waveform(QString title, int sampleCount, QObject* parent)
    : QObject(parent)
    , m_waveformChart(new QChart)
    , m_waveformSeries(new QLineSeries)
    , m_waveformChartView(new QChartView(m_waveformChart))
    , m_axisX(new QValueAxis)
    , m_axisY(new QValueAxis)
    , m_sampleCount(DEFAULT_SAMPLE_COUNT)
{
    m_sampleCount = sampleCount;
    m_waveformChartView->resize(800, 600);
    m_waveformChartView->setMinimumSize(380, 300);
    m_waveformChart->addSeries(m_waveformSeries);
    QValueAxis* axisX = new QValueAxis;
    axisX->setRange(0, sampleCount);
    axisX->setLabelFormat("%g");
    axisX->setTitleText("Samples");
    QValueAxis* axisY = new QValueAxis;
    axisY->setRange(-1, 1);
    axisY->setTitleText("Amplitude");
    m_waveformChart->addAxis(axisX, Qt::AlignBottom);
    m_waveformSeries->attachAxis(axisX);
    m_waveformChart->addAxis(axisY, Qt::AlignLeft);
    m_waveformSeries->attachAxis(axisY);
    m_waveformChart->legend()->hide();
}

QChartView* Waveform::getChartView()
{
    return m_waveformChartView;
}

QLineSeries* Waveform::getSeries()
{
    return m_waveformSeries;
}

QChart* Waveform::getChart()
{
    return m_waveformChart;
}

QValueAxis* Waveform::getAxisX()
{
    return m_axisX;
}

QValueAxis* Waveform::getAxisY()
{
    return m_axisY;
}

int Waveform::getSampleCount()
{
    return m_sampleCount;
}