#include "DFTWorkerThread.h"
#include <math.h>

DFTWorkerThread::DFTWorkerThread()
    : m_dataBuffer(nullptr)
{
    qRegisterMetaType<QVector<QPointF>>("QVector<QPointF>");
}

DFTWorkerThread::~DFTWorkerThread()
{

}

void DFTWorkerThread::setDataBuffer(const QBuffer* dataBuffer)
{
    m_dataBuffer = dataBuffer;
}

void DFTWorkerThread::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void DFTWorkerThread::clearData()
{
    m_spectrumBuffer.clear();
}

void DFTWorkerThread::run()
{
    m_spectrumBuffer.clear();

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    if (N == 0)
    {
        return;
    }

    //qDebug() << "DFTWorkerThread::run() Number of samples received: " << N;

    // Get raw data and define K
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;
    const int K = Constants::MAX_FREQUENCY;

    m_spectrumBuffer.reserve(K - Constants::MIN_FREQUENCY);

    double maxSum = 0.0;
    std::complex<double> currentSum;

    std::vector<std::pair<ulong, std::complex<double>>> output;
    output.reserve(N);

    const ulong samplesPerSec = m_format.bytesForDuration(1e6) / (m_format.sampleSize() / 8);

    // Loop through each k
    for (ulong k = 0; k < N; ++k)
    {
        currentSum = std::complex<double>(0, 0);

        // Loop through each sample n
        for (ulong n = 0; n < N; ++n)
        {
            if (isInterruptionRequested())
            {
                clearData();
                return;
            }

            double xn = data_short[n];
            double real = std::cos(((2 * M_PI) / samplesPerSec) * k * n);
            double imag = std::sin(((2 * M_PI) / samplesPerSec) * k * n);
            std::complex<double> w (real, -imag);
            currentSum += xn * w;
        }

        double mag = std::abs(currentSum);

        // Keep track of largest y-value seen so far
        if (mag > maxSum)
            maxSum = mag;

        // Add point to output buffer
        output.push_back(std::make_pair(k, currentSum));
    }

    // Normalize y values
    for (size_t i = 0; i < (output.size() / 2); ++i)
    {
        // Only plot the frequencies we're interested in
        if (output[i].first > Constants::MAX_FREQUENCY)
            break;
        else if (output[i].first < Constants::MIN_FREQUENCY)
            continue;

        double abs = std::abs(output[i].second) / maxSum;
        QPointF point(output[i].first, abs);
        m_spectrumBuffer.append(point);
    }

    emit resultReady(m_spectrumBuffer);
}
