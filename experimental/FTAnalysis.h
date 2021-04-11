#ifndef FTANALYSIS_H
#define FTANALYSIS_H

#include "FTController.h"

#include <iostream>
#include <QObject>
#include <QVector>
#include <QPointF>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QFile>
#include <QSignalSpy>
#include <QDebug>

class FTAnalysis : public QObject
{
    Q_OBJECT

public:
    explicit FTAnalysis(QObject *parent = nullptr);

public slots:
    void startPerformanceAnalysis();
signals:
    void finished();

private:
    FTController m_ftController;
    QAudioDecoder m_decoder;
    QFile m_file;
    QAudioFormat m_format;
    double m_avgTime;

    void startDecoder(const QString& filePath);
    void startTrials(const QString& filePath, const char* slot);

private slots:
    void receiveFTResults(const QVector<QPointF> points, const double elapsedSeconds);
    void writeAudioDataToBuffer();
    void calcFFT();
    void calcDistributedFFT();
};

#endif // FTANALYSIS_H
