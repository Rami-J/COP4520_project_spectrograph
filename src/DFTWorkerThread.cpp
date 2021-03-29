#include "DFTWorkerThread.h"
#include <math.h>

DFTWorkerThread::DFTWorkerThread()
    : m_dataBuffer(new QBuffer)
{
    qRegisterMetaType<QVector<QPointF>>("QVector<QPointF>");
    m_dataBuffer->open(QIODevice::ReadWrite);
}

DFTWorkerThread::~DFTWorkerThread()
{
    m_dataBuffer->close();
    m_dataBuffer->setData(nullptr);
}

QBuffer* DFTWorkerThread::getDataBuffer()
{
    return m_dataBuffer;
}

void DFTWorkerThread::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void DFTWorkerThread::clearData()
{
    m_dataBuffer->close();
    m_dataBuffer->setData(nullptr);
    m_dataBuffer->open(QIODevice::ReadWrite);
    m_spectrumBuffer.clear();
}

void DFTWorkerThread::run()
{
    m_spectrumBuffer.clear();

    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    if (N == 0)
    {
        return;
    }

    qDebug() << "DFTWorkerThread::run() Number of samples received: " << N;

    // Get raw data and define K
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;
    const int K = Constants::MAX_FREQUENCY;

    m_spectrumBuffer.reserve(K - Constants::MIN_FREQUENCY);

    double currentSum = 0.0;
    double maxSum = 0.0;

    // Loop through each k
    for (int k = Constants::MIN_FREQUENCY; k < K; ++k)
    {
        currentSum = 0.0;

        // Loop through each sample n
        for (ulong n = 0; n < N; ++n)
        {
            if (isInterruptionRequested())
            {
                clearData();
                return;
            }

            double xn = data_short[n];
            double real = xn * std::cos(((2 * M_PI) / N) * k * n);
            currentSum += real;
        }

        currentSum = std::abs(currentSum);

        // Keep track of largest y-value seen so far
        if (currentSum > maxSum)
            maxSum = currentSum;

        // Add point to chart buffer
        m_spectrumBuffer.append(QPointF(k, currentSum));
    }

    // Normalize y values
    for (int i = 0; i < m_spectrumBuffer.size(); ++i)
    {
        m_spectrumBuffer[i].setY(m_spectrumBuffer[i].y() / maxSum);
    }

    emit resultReady(m_spectrumBuffer);
}
