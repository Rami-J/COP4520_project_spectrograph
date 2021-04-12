#ifndef FTCONTROLLER_H
#define FTCONTROLLER_H

#include "Constants.h"
#include "DFTWorkerThread.h"
#include "DistributedDFTWorkerThread.h"
#include "FFTWorkerThread.h"
#include "DistributedFFTWorkerThread.h"

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
*   worker classes.
*  
*/
class FTController : public QObject
{
    Q_OBJECT

public:
    FTController();
    ~FTController();
    void startDFTInAThread(const QAudioFormat format);
    void startDistributedDFT(const QAudioFormat format);
    void startFFTInAThread(const QAudioFormat format);
    void startDistributedFFT(const QAudioFormat format);
    QBuffer* getDataBuffer();
    void setAudioFormat(QAudioFormat);
    void clear();

signals:
    void spectrumDataReady(const QVector<QPointF> points, const double elapsedSeconds);

public slots:
    void handleResults(const QVector<QPointF> points);
    void handleDistributedDFTResults(const QVector<QPointF> points, const int workerID);
    void handleDistributedFFTResults(const QVector<QPointF> points, const int workerID);

private:
    DistributedDFTWorkerThread* m_DistributedDFTWorkerThreads[Constants::NUM_DFT_WORKERS];
    DistributedFFTWorkerThread* m_DistributedFFTWorkerThreads[Constants::NUM_FFT_WORKERS];
    DFTWorkerThread* m_DFTWorkerThread;
    FFTWorkerThread* m_FFTWorkerThread;
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
