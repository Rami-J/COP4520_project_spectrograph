#ifndef DISTRIBUTEDFFTWORKERTHREAD_H
#define DISTRIBUTEDFFTWORKERTHREAD_H

#include "Constants.h"
#include "FFTUtils.h"

#include <complex>
#include <atomic>

#include <QDebug>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCore/QBuffer>
#include <QAudioFormat>

#define _USE_MATH_DEFINES

class DistributedFFTWorkerThread : public QThread
{
    Q_OBJECT

        void run() override;

public:
    DistributedFFTWorkerThread();
    ~DistributedFFTWorkerThread();

    void setAudioFormat(QAudioFormat format);
    void setWorkerID(int workerID);

    static double getMaxSum();
    static void setMaxSum(double sum);
    int getWorkerID();
    void setDataBuffer(const QBuffer* dataBuffer);

    void clearData();

signals:
    void distributedResultReady(const QVector<QPointF> points, const int workerID);

private:
    const QBuffer* m_dataBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QAudioFormat m_format;
    int m_workerID;

    /*
    * Computes the discrete Fourier transform (FFT) of the given real/imaginary vectors,
    * using the Cooley-Tukey decimation-in-time radix-2 algorithm.
    * If the vectors are not a power of 2, they are padded to the next highest power of 2.
    * 
    * This function is almost the same as FFTWorkerThread::cooleyTukey, except normalization of
    * amplitude is deferred to slot FTController::handleDistributedFFTResults().
    *
    * Returns a vector of the (frequency_bin, amplitude) output.
    */
    std::vector<std::pair<size_t, double>> cooleyTukey(std::vector<double>& real, std::vector<double>& imag);
};

#endif // DISTRIBUTEDFFTWORKERTHREAD_H