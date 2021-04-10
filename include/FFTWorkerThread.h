#ifndef FFTWORKERTHREAD_H
#define FFTWORKERTHREAD_H

#include "Constants.h"
#include "FFTUtils.h"

#include <complex>
#include <vector>

#include <QDebug>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCore/QBuffer>
#include <QAudioFormat>

#define _USE_MATH_DEFINES

class FFTWorkerThread : public QThread
{
    Q_OBJECT
        void run() override;

public:
    FFTWorkerThread();
    ~FFTWorkerThread();

    void setAudioFormat(QAudioFormat format);

    void setDataBuffer(const QBuffer* dataBuffer);

    void clearData();

signals:
    void resultReady(const QVector<QPointF> points);

private:
    const QBuffer* m_dataBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QAudioFormat m_format;


    /*
     * Computes the discrete Fourier transform (FFT) of the given real/imaginary vectors,
     * using the Cooley-Tukey decimation-in-time radix-2 algorithm.
     * If the vectors are not a power of 2, they are padded to the next highest power of 2.
     *
     * Returns a vector of the normalized frequency amplitude output.
     */
    std::vector<std::pair<size_t, double>> cooleyTukey(std::vector<double> &real, std::vector<double> &imag);
};

#endif // FFTWORKERTHREAD_H
