#include "Spectrograph.h"
#include <math.h>

static const int MAX_FREQUENCY = 1000;
static const int MIN_FREQUENCY = 100;

Spectrograph::Spectrograph(QString title, QObject* parent) 
	: QObject(parent)
	, m_spectrumChart(new QChart)
	, m_spectrumSeries(new QLineSeries)
    , m_spectrumChartView(new QChartView(m_spectrumChart))
    , m_axisX(new QValueAxis)
    , m_axisY(new QValueAxis)
    , m_dataBuffer(new QBuffer)
{
    m_dataBuffer->open(QIODevice::ReadWrite);
    m_spectrumChartView->resize(800, 600);
    m_spectrumChartView->setMinimumSize(380, 300);
    m_spectrumChart->addSeries(m_spectrumSeries);
    m_axisX->setRange(MIN_FREQUENCY, MAX_FREQUENCY);
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

QBuffer* Spectrograph::getDataBuffer()
{
    return m_dataBuffer;
}

void Spectrograph::clear()
{
    // Reset the audio data buffer
    m_dataBuffer->setBuffer(nullptr);
}

void Spectrograph::calculateDFT(const QAudioFormat format, const qreal peakVal)
{
    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (format.sampleSize() / 8);

    // Get raw data and define K
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;
    const int K = MAX_FREQUENCY;

    QVector<QPointF> spectrumBuffer;
    spectrumBuffer.reserve(K);

    double currentSum = 0.0;
    double maxSum = 0.0;

    // Loop through each k
    for (int k = MIN_FREQUENCY; k < K; ++k)
    {
        currentSum = 0.0;

        // Loop through each sample n
        for (int n = 0; n < N; ++n)
        {
            double xn = data_short[n];
            double real = xn * std::cos(((2 * M_PI) / N) * k * n);
            currentSum += real;
        }

        // Keep track of largest y-value seen so far
        if (currentSum > maxSum)
            maxSum = currentSum;

        // Add point to chart buffer
        spectrumBuffer.append(QPointF(k, currentSum));
    }
    
    // Normalize y values
    for (int i = 0; i < spectrumBuffer.size(); ++i)
    {
        spectrumBuffer[i].setY(spectrumBuffer[i].y() / maxSum);
    }

    m_spectrumSeries->replace(spectrumBuffer);
}