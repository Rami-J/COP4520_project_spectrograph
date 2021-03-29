#include "DistributedDFTWorkerThread.h"

#include <math.h>

DistributedDFTWorkerThread::DistributedDFTWorkerThread()
    : m_dataBuffer(new QBuffer)
    , m_workerID(0)
{

}

DistributedDFTWorkerThread::~DistributedDFTWorkerThread()
{
    m_dataBuffer->close();
    m_dataBuffer->setData(nullptr);
}

int DistributedDFTWorkerThread::getWorkerID()
{
    return m_workerID;
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
    m_dataBuffer->close();
    m_dataBuffer->setData(nullptr);
    m_dataBuffer->open(QIODevice::ReadWrite);
    m_spectrumBuffer.clear();
}

void DistributedDFTWorkerThread::run()
{
    // TODO

    qDebug() << "DistributedDFTWorkerThread::run() workerID is " << m_workerID;

    if (isInterruptionRequested())
    {
        clearData();
        return;
    }

    emit distributedResultReady(m_spectrumBuffer, m_workerID);
}