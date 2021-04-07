#include "FTController.h"

FTController::FTController()
    : m_dataBuffer(new QBuffer)
    , m_DFTWorkerThread(new DFTWorkerThread)
    , m_numWorkersFinished(0)
{
    m_combinedPoints.resize((Constants::MAX_FREQUENCY - Constants::MIN_FREQUENCY + 1));

    m_dataBuffer->open(QIODevice::ReadWrite);
    m_DFTWorkerThread->setDataBuffer(m_dataBuffer);
    connect(m_DFTWorkerThread, &DFTWorkerThread::resultReady, this, &FTController::handleResults);

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i] = new DistributedDFTWorkerThread;
        m_DistributedDFTWorkerThreads[i]->setWorkerID(i);
        m_DistributedDFTWorkerThreads[i]->setDataBuffer(m_dataBuffer);
        connect(m_DistributedDFTWorkerThreads[i], &DistributedDFTWorkerThread::distributedResultReady, this, &FTController::handleDistributedResults);
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

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        if (m_DistributedDFTWorkerThreads[i]->isRunning())
        {
            m_DistributedDFTWorkerThreads[i]->requestInterruption();
            m_DistributedDFTWorkerThreads[i]->wait();
        }
    }
}

void FTController::resetThreadData()
{
    m_DFTWorkerThread->clearData();

    for (int i = 0; i < Constants::NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i]->clearData();
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
    m_DFTWorkerThread->setAudioFormat(format);
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

void FTController::handleResults(const QVector<QPointF> points)
{
    emit spectrumDataReady(points);

    m_timeEnd = std::chrono::high_resolution_clock::now();

    /* Getting number of seconds as a double. */
    std::chrono::duration<double> elapsedSeconds = m_timeEnd - m_timeStart;
    qDebug() << "FTController::startDFTInAThread() Total Elapsed Time (s): " << elapsedSeconds.count();
}

void FTController::handleDistributedResults(const QVector<QPointF> points, const int workerID)
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
            qDebug() << "DistributedDFTWorkerThread Point is " << m_combinedPoints[i];
        }

        m_numWorkersFinished = 0;
        DistributedDFTWorkerThread::setMaxSum(0.0);
        emit spectrumDataReady(m_combinedPoints);

        m_timeEnd = std::chrono::high_resolution_clock::now();

        /* Getting number of seconds as a double. */
        std::chrono::duration<double> elapsedSeconds = m_timeEnd - m_timeStart;
        qDebug() << "FTController::startDistributedDFT() Total Elapsed Time (s): " << elapsedSeconds.count();
    }
}
