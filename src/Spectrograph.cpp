#include "Spectrograph.h"

Spectrograph::Spectrograph(QString title, QObject* parent) 
	: QObject(parent)
	, m_spectrumChart(new QChart)
	, m_spectrumSeries(new QLineSeries)
    , m_spectrumChartView(new QChartView(m_spectrumChart))
    , m_axisX(new QValueAxis)
    , m_axisY(new QValueAxis)
    , m_FTController(new FTController)
{
    m_spectrumChartView->resize(800, 600);
    m_spectrumChartView->setMinimumSize(380, 300);
    m_spectrumChart->addSeries(m_spectrumSeries);
    m_axisX->setRange(Constants::MIN_FREQUENCY, Constants::MAX_FREQUENCY);
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Frequency (Hz)");
    m_axisY = new QValueAxis;
    m_axisY->setRange(0, 1);
    m_axisY->setTitleText("Amplitude");
    m_spectrumChart->addAxis(m_axisX, Qt::AlignBottom);
    m_spectrumSeries->attachAxis(m_axisX);
    m_spectrumChart->addAxis(m_axisY, Qt::AlignLeft);
    m_spectrumSeries->attachAxis(m_axisY);
    m_spectrumChart->legend()->hide();
    m_spectrumChart->setTitle(title);

    connect(m_FTController, &FTController::spectrumDataReady, this, &Spectrograph::plotSpectrumData);
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

QBuffer* Spectrograph::getDataBuffer()
{
    return m_FTController->getDataBuffer();
}

void Spectrograph::cancelCalculation()
{
    // Reset the audio data buffer
    m_FTController->clear();
}

void Spectrograph::calculateDFT(const QAudioFormat format)
{
    // Comment first line and uncomment second line to perform distributed DFT calculation
    //m_FTController->startDFTInAThread(format);
    m_FTController->startDistributedDFT(format);
}

void Spectrograph::plotSpectrumData(const QVector<QPointF> points)
{
    qDebug() << "Spectrograph::plotSpectrumData() plotting " << points.size() << " points";

    // Waiting to replace all the points on the graph at once is more efficient than constantly
    // appending the data points as it's being computed.
    m_spectrumSeries->replace(points);
}
