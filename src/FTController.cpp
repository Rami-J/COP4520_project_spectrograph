#include "FTController.h"

FTController::FTController()
    : m_dataBuffer(new QBuffer)
    , m_DFTWorkerThread(new DFTWorkerThread)
    , m_FFTWorkerThread(new FFTWorkerThread)
    , m_numWorkersFinished(0)
{
    m_combinedPoints.resize((Constants::MAX_FREQUENCY - Constants::MIN_FREQUENCY + 1));

    m_dataBuffer->open(QIODevice::ReadWrite);
    m_DFTWorkerThread->setDataBuffer(m_dataBuffer);
    m_FFTWorkerThread->setDataBuffer(m_dataBuffer);
    connect(m_DFTWorkerThread, &DFTWorkerThread::resultReady, this, &FTController::handleResults);
    connect(m_FFTWorkerThread, &FFTWorkerThread::resultReady, this, &FTController::handleResults);

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i] = new DistributedDFTWorkerThread;
        m_DistributedDFTWorkerThreads[i]->setWorkerID(i);
        m_DistributedDFTWorkerThreads[i]->setDataBuffer(m_dataBuffer);
        connect(m_DistributedDFTWorkerThreads[i], &DistributedDFTWorkerThread::distributedResultReady, this, &FTController::handleDistributedDFTResults);
    }

    for (int i = 0; i < Constants::NUM_FFT_WORKERS; ++i)
    {
        m_DistributedFFTWorkerThreads[i] = new DistributedFFTWorkerThread;
        m_DistributedFFTWorkerThreads[i]->setWorkerID(i);
        m_DistributedFFTWorkerThreads[i]->setDataBuffer(m_dataBuffer);
        connect(m_DistributedFFTWorkerThreads[i], &DistributedFFTWorkerThread::distributedResultReady, this, &FTController::handleDistributedFFTResults);
    }
}

FTController::~FTController() 
{
    terminateRunningThreads();
}

void FTController::terminateRunningThreads()
{
    if (m_DFTWorkerThread->isRunning())
    {
        m_DFTWorkerThread->requestInterruption();
        m_DFTWorkerThread->wait();
    }

    if (m_FFTWorkerThread->isRunning())
    {
        m_FFTWorkerThread->requestInterruption();
        m_FFTWorkerThread->wait();
    }

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        if (m_DistributedDFTWorkerThreads[i]->isRunning())
        {
            m_DistributedDFTWorkerThreads[i]->requestInterruption();
            m_DistributedDFTWorkerThreads[i]->wait();
        }
    }

    for (int i = 0; i < Constants::NUM_FFT_WORKERS; ++i)
    {
        if (m_DistributedFFTWorkerThreads[i]->isRunning())
        {
            m_DistributedFFTWorkerThreads[i]->requestInterruption();
            m_DistributedFFTWorkerThreads[i]->wait();
        }
    }
}

void FTController::resetThreadData()
{
    m_DFTWorkerThread->clearData();
    m_FFTWorkerThread->clearData();

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i]->clearData();
    }

    for (int i = 0; i < Constants::NUM_FFT_WORKERS; ++i)
    {
        m_DistributedFFTWorkerThreads[i]->clearData();
    }

    // Clear data buffer
    m_dataBuffer->close();
    m_dataBuffer->setData(nullptr);
    m_dataBuffer->open(QIODevice::ReadWrite);
}

void FTController::clear()
{
    terminateRunningThreads();
    resetThreadData();
}

QBuffer* FTController::getDataBuffer()
{
    return m_dataBuffer;
}

void FTController::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void FTController::startDFTInAThread(const QAudioFormat format)
{
    m_timeStart = std::chrono::high_resolution_clock::now();

    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    m_DFTWorkerThread->setAudioFormat(format);
    m_DFTWorkerThread->start();
}

void FTController::startDistributedDFT(const QAudioFormat format)
{
    m_timeStart = std::chrono::high_resolution_clock::now();

    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i]->setAudioFormat(format);
        m_DistributedDFTWorkerThreads[i]->start();
    }
}

void FTController::startFFTInAThread(const QAudioFormat format)
{
    m_timeStart = std::chrono::high_resolution_clock::now();

    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    m_FFTWorkerThread->setAudioFormat(format);
    m_FFTWorkerThread->start();
}

void FTController::startDistributedFFT(const QAudioFormat format)
{
    m_timeStart = std::chrono::high_resolution_clock::now();

    // Reset data buffer to position 0
    m_dataBuffer->seek(0);

    for (int i = 0; i < Constants::NUM_FFT_WORKERS; ++i)
    {
        m_DistributedFFTWorkerThreads[i]->setAudioFormat(format);
        m_DistributedFFTWorkerThreads[i]->start();
    }
}

void FTController::handleResults(const QVector<QPointF> points)
{
    m_timeEnd = std::chrono::high_resolution_clock::now();

    /* Getting number of seconds as a double. */
    std::chrono::duration<double> elapsedSeconds = m_timeEnd - m_timeStart;
    //qDebug() << "FTController Total Elapsed Time (s): " << elapsedSeconds.count();

    emit spectrumDataReady(points, elapsedSeconds.count());

}

void FTController::handleDistributedDFTResults(const QVector<QPointF> points, const int workerID)
{
    Q_UNUSED(workerID);

    for (int i = 0; i < points.size(); ++i)
    {
        int index = points[i].x() - Constants::MIN_FREQUENCY;
        m_combinedPoints[index] = points[i];
    }

    m_numWorkersFinished++;

    if (m_numWorkersFinished == Constants::NUM_DFT_WORKERS)
    {
        // Normalize y values
        for (int i = 0; i < m_combinedPoints.size(); ++i)
        {
            m_combinedPoints[i].setY(m_combinedPoints[i].y() / DistributedDFTWorkerThread::getMaxSum());
        }

        m_numWorkersFinished = 0;
        DistributedDFTWorkerThread::setMaxSum(0.0);

        m_timeEnd = std::chrono::high_resolution_clock::now();

        /* Getting number of seconds as a double. */
        std::chrono::duration<double> elapsedSeconds = m_timeEnd - m_timeStart;
        //qDebug() << "FTController::startDistributedDFT() Total Elapsed Time (s): " << elapsedSeconds.count();
        emit spectrumDataReady(m_combinedPoints, elapsedSeconds.count());
    }
}

void FTController::handleDistributedFFTResults(const QVector<QPointF> points, const int workerID)
{
    for (int i = 0; i < points.size(); ++i)
    {
        int index = points[i].x() - Constants::MIN_FREQUENCY;
        m_combinedPoints[index] = points[i];
    }

    //qDebug() << "FTController::handleDistributedFFTResults() worker " << workerID << " finished. Received " << points.size() << " points";

    m_numWorkersFinished++;

    if (m_numWorkersFinished == Constants::NUM_FFT_WORKERS)
    {
        // Normalize y values
        for (int i = 0; i < m_combinedPoints.size(); ++i)
        {
            m_combinedPoints[i].setX(i + Constants::MIN_FREQUENCY);
            m_combinedPoints[i].setY(m_combinedPoints[i].y() / DistributedFFTWorkerThread::getMaxSum());
        }

        m_numWorkersFinished = 0;
        DistributedFFTWorkerThread::setMaxSum(0.0);

        m_timeEnd = std::chrono::high_resolution_clock::now();

        /* Getting number of seconds as a double. */
        std::chrono::duration<double> elapsedSeconds = m_timeEnd - m_timeStart;
        //qDebug() << "FTController::startDistributedFFT() Total Elapsed Time (s): " << elapsedSeconds.count();
        emit spectrumDataReady(m_combinedPoints, elapsedSeconds.count());
    }
}
