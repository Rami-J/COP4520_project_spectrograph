#include "DistributedDFTWorkerThread.h"

#include <math.h>

static std::atomic<double> maxSum{ 0 };

DistributedDFTWorkerThread::DistributedDFTWorkerThread()
    : m_dataBuffer(nullptr)
    , m_workerID(0)
{

}

DistributedDFTWorkerThread::~DistributedDFTWorkerThread()
{

}

int DistributedDFTWorkerThread::getWorkerID()
{
    return m_workerID;
}

double DistributedDFTWorkerThread::getMaxSum()
{
    return maxSum;
}

void DistributedDFTWorkerThread::setMaxSum(double sum)
{
    maxSum = sum;
}

void DistributedDFTWorkerThread::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void DistributedDFTWorkerThread::setWorkerID(int workerID)
{
    m_workerID = workerID;
}

void DistributedDFTWorkerThread::clearData()
{
    m_spectrumBuffer.clear();
}

void DistributedDFTWorkerThread::setDataBuffer(const QBuffer* dataBuffer)
{
    m_dataBuffer = dataBuffer;
}

void DistributedDFTWorkerThread::run()
{
    m_spectrumBuffer.clear();

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    if (N == 0)
    {
        return;
    }

    // Get raw data
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;

    // range calculation for current worker
    ulong k_start = (m_workerID * N) / Constants::NUM_DFT_WORKERS;
    ulong k_end = ((m_workerID + 1) * N) / Constants::NUM_DFT_WORKERS;

    if (k_end < N && (m_workerID + 1) == Constants::NUM_DFT_WORKERS)
        k_end = N;

    qDebug() << "DistributedDFTWorkerThread::run() Worker ID: " << m_workerID << " Number of samples processing: " << (k_end - k_start + 1);

    m_spectrumBuffer.reserve(Constants::MAX_FREQUENCY - Constants::MIN_FREQUENCY);

    std::complex<double> currentSum;

    std::vector<std::pair<ulong, std::complex<double>>> output;
    output.reserve(N);

    const ulong samplesPerSec = m_format.bytesForDuration(1e6) / (m_format.sampleSize() / 8);

    // Loop through each k
    for (ulong k = k_start; k < k_end; ++k)
    {
        currentSum = std::complex<double>(0, 0);

        // Loop through each sample n
        for (ulong n = k_start; n < k_end; ++n)
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

        double mag = std::fabs(currentSum);

        // Keep track of largest y-value seen so far
        if (mag > maxSum)
            maxSum = mag;

        // Add point to output buffer
        output.push_back(std::make_pair(k, currentSum));
    }

    for (size_t i = 0; i < (output.size() / 2); ++i)
    {
        // Only plot the frequencies we're interested in
        if (output[i].first > Constants::MAX_FREQUENCY)
            break;
        else if (output[i].first < Constants::MIN_FREQUENCY)
            continue;

        double abs = std::fabs(output[i].second);
        QPointF point(output[i].first, abs);
        m_spectrumBuffer.append(point);
    }

    emit distributedResultReady(m_spectrumBuffer, m_workerID);
}
