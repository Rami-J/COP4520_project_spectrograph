#ifndef DISTRIBUTEDFFTWORKERTHREAD_H
#define DISTRIBUTEDFFTWORKERTHREAD_H

#include "Constants.h"

#include <complex>
#include <atomic>

#include <QDebug>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCore/QBuffer>
#include <QAudioFormat>

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
};

#endif // DISTRIBUTEDFFTWORKERTHREAD_H