#ifndef DFTWORKERTHREAD_H
#define DFTWORKERTHREAD_H

#include "Constants.h"

#include <QDebug>
#include <QtCore/QThread>
#include <QtCore/QObject>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCore/QBuffer>
#include <QAudioFormat>

#define _USE_MATH_DEFINES

class DFTWorkerThread : public QThread
{
    Q_OBJECT
        void run() override;

public:
    DFTWorkerThread();
    ~DFTWorkerThread();

    void setAudioFormat(QAudioFormat format);

    void setDataBuffer(const QBuffer* dataBuffer);

    void clearData();

signals:
    void resultReady(const QVector<QPointF> points);

private:
    const QBuffer* m_dataBuffer;
    QVector<QPointF> m_spectrumBuffer;
    QAudioFormat m_format;
};

#endif // DFTWORKERTHREAD_H