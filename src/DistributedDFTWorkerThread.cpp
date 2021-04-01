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
    // TODO: further split up the N samples of work between threads?

    m_spectrumBuffer.clear();

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    if (N == 0)
    {
        return;
    }

    qDebug() << "DistributedDFTWorkerThread::run() Worker ID: " << m_workerID << " Number of samples processing: " << N;

    // Get raw data
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;

    // k range calculation for current worker
    const int num_frequency_bins = (Constants::MAX_FREQUENCY - Constants::MIN_FREQUENCY);
    const int k_start = (m_workerID * num_frequency_bins) / Constants::NUM_DFT_WORKERS + Constants::MIN_FREQUENCY;
    const int k_end = ((m_workerID + 1) * num_frequency_bins) / Constants::NUM_DFT_WORKERS + Constants::MIN_FREQUENCY;

    m_spectrumBuffer.reserve(k_end - k_start);

    double currentSum = 0.0;
    ulong samplesPerSec = m_format.bytesForDuration(1e6) / (m_format.sampleSize() / 8);

    // Loop through each k
    for (int k = k_start; k < k_end; ++k)
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
            double real = xn * std::cos(((2 * M_PI) / samplesPerSec) * k * n);
            currentSum += real;
        }

        currentSum = std::fabs(currentSum);

        // Keep track of largest y-value seen so far
        if (currentSum > maxSum)
            maxSum = currentSum;

        // Add point to chart buffer
        m_spectrumBuffer.append(QPointF(k, currentSum));
    }

    emit distributedResultReady(m_spectrumBuffer, m_workerID);
}
