#ifndef FTCONTROLLER_H
#define FTCONTROLLER_H

#include "Constants.h"
#include "DFTWorkerThread.h"
#include "DistributedDFTWorkerThread.h"

#include <atomic>
#include <chrono>

#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QAudioFormat>

/**
*   Fourier Transform Controller (FTController) handles calculation of DFT/FFT
*   asynchronously on a thread separate from the main GUI thread, through the use of the
*   DFTWorker class.
*  
*   TODO: perform distributed computation of DFT by using multiple DFT worker threads, create an FFTWorker as well
*         for comparison between DFT and FFT
*/
class FTController : public QObject
{
    Q_OBJECT

public:
    FTController();
    ~FTController();
    void startDFTInAThread(const QAudioFormat format);
    void startDistributedDFT(const QAudioFormat format);
    QBuffer* getDataBuffer();
    void setAudioFormat(QAudioFormat);
    void clear();

signals:
    void spectrumDataReady(const QVector<QPointF> points);
    void distributedSpectrumDataReady(const QVector<QPointF> points);

public slots:
    void handleResults(const QVector<QPointF> points);
    void handleDistributedResults(const QVector<QPointF> points, const int workerID);

private:
    DistributedDFTWorkerThread* m_DistributedDFTWorkerThreads[Constants::NUM_DFT_WORKERS];
    DFTWorkerThread* m_DFTWorkerThread;
    QAudioFormat m_format;
    QBuffer* m_dataBuffer;

    QVector<QPointF> m_combinedPoints;
    std::atomic<int> m_numWorkersFinished;

    std::chrono::high_resolution_clock::time_point m_timeStart;
    std::chrono::high_resolution_clock::time_point m_timeEnd;

    void terminateRunningThreads();
    void resetThreadData();
};

#endif // FTCONTROLLER_H
