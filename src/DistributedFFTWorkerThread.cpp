#include "DistributedFFTWorkerThread.h"

#include <math.h>

static std::atomic<double> maxSum{ 0 };

DistributedFFTWorkerThread::DistributedFFTWorkerThread()
    : m_dataBuffer(nullptr)
    , m_workerID(0)
{

}

DistributedFFTWorkerThread::~DistributedFFTWorkerThread()
{

}

int DistributedFFTWorkerThread::getWorkerID()
{
    return m_workerID;
}

double DistributedFFTWorkerThread::getMaxSum()
{
    return maxSum;
}

void DistributedFFTWorkerThread::setMaxSum(double sum)
{
    maxSum = sum;
}

void DistributedFFTWorkerThread::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void DistributedFFTWorkerThread::setWorkerID(int workerID)
{
    m_workerID = workerID;
}

void DistributedFFTWorkerThread::clearData()
{
    m_spectrumBuffer.clear();
}

void DistributedFFTWorkerThread::setDataBuffer(const QBuffer* dataBuffer)
{
    m_dataBuffer = dataBuffer;
}

void DistributedFFTWorkerThread::run()
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
    ulong n_start = (m_workerID * N) / Constants::NUM_FFT_WORKERS;
    ulong n_end = ((m_workerID + 1) * N) / Constants::NUM_FFT_WORKERS;

    if (n_end < N && (m_workerID + 1) == Constants::NUM_FFT_WORKERS)
        n_end = N;

    qDebug() << "DistributedFFTWorkerThread::run() Worker ID: " << m_workerID << " Number of samples processing: " << (n_start - n_end + 1);

    m_spectrumBuffer.reserve(Constants::MAX_FREQUENCY - Constants::MIN_FREQUENCY);

    emit distributedResultReady(m_spectrumBuffer, m_workerID);
}