#include "FTController.h"

FTController::FTController()
    : m_DFTWorkerThread(new DFTWorkerThread)
{

}

FTController::~FTController() 
{
    //m_FTWorkerThread->quit();
    //m_FTWorkerThread->wait();
}

void FTController::clear()
{
    if (m_DFTWorkerThread->isRunning())
    {
        m_DFTWorkerThread->requestInterruption();
        m_DFTWorkerThread->wait();
    }
    m_DFTWorkerThread->clearData();
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
    connect(m_DFTWorkerThread, &DFTWorkerThread::resultReady, this, &FTController::handleResults);
    //connect(m_DFTWorkerThread, &DFTWorkerThread::finished, m_DFTWorkerThread, &QObject::deleteLater);
    m_DFTWorkerThread->start();
}

void FTController::handleResults(const QVector<QPointF> points)
{
    emit spectrumDataReady(points);
}