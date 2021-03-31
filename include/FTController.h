#ifndef FTCONTROLLER_H
#define FTCONTROLLER_H

#include "DFTWorkerThread.h"
#include "DistributedDFTWorkerThread.h"

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

    // Define number of DFT workers to 9 since we can split up the k-frequency calculations evenly
    // because MAX_FREQUENCY - MIN_FREQUENCY = 1000 - 100 = 900 and 900 / 9 = 100 frequency bins per thread
    const static int NUM_DFT_WORKERS = 9;

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

public slots:
    void handleResults(const QVector<QPointF> points);
    void handleDistributedResults(const QVector<QPointF> points, const int workerID);

private:
    DistributedDFTWorkerThread* m_DistributedDFTWorkerThreads[NUM_DFT_WORKERS];
    DFTWorkerThread* m_DFTWorkerThread;
    QAudioFormat m_format;

    void terminateRunningThreads();
    void resetThreadData();
};

#endif // FTCONTROLLER_H