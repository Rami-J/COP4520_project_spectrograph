#include "FTController.h"

FTController::FTController()
    : m_DFTWorkerThread(new DFTWorkerThread)
{
    connect(m_DFTWorkerThread, &DFTWorkerThread::resultReady, this, &FTController::handleResults);

    for (int i = 0; i < NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i] = new DistributedDFTWorkerThread;
        m_DistributedDFTWorkerThreads[i]->setWorkerID(i);
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

    for (int i = 0; i < NUM_DFT_WORKERS; ++i)
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

    for (int i = 0; i < NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i]->clearData();
    }
}

void FTController::clear()
{
    terminateRunningThreads();
    resetThreadData();
}

QBuffer* FTController::getDataBuffer()
{
    return m_DFTWorkerThread->getDataBuffer();
}

void FTController::setAudioFormat(QAudioFormat format)
{
    m_format = format;
    m_DFTWorkerThread->setAudioFormat(format);
}

void FTController::startDFTInAThread(const QAudioFormat format)
{
    m_DFTWorkerThread->setAudioFormat(format);
    m_DFTWorkerThread->start();
}

void FTController::startDistributedDFT(const QAudioFormat format)
{
    for (int i = 0; i < NUM_DFT_WORKERS; ++i)
    {
        m_DistributedDFTWorkerThreads[i]->setAudioFormat(format);
        m_DistributedDFTWorkerThreads[i]->start();
    }
}

void FTController::handleResults(const QVector<QPointF> points)
{
    emit spectrumDataReady(points);
}

void FTController::handleDistributedResults(const QVector<QPointF> points, const int workerID)
{
    qDebug() << "received " << points.size() << " from worker " << workerID;
}