#pragma once

#include <QDebug>
#include <QIODevice>
#include <QBuffer>
#include <QAudioDecoder>
#include <QAudioFormat>
#include <QFile>
#include <QtCore/QPointF>
#include <QtCore/QVector>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QXYSeries>

QT_CHARTS_BEGIN_NAMESPACE
class QXYSeries;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

// Class to decode audio files and push the data to an output device. It also manages
// media states such as playing, paused.
class AudioFileStream : public QIODevice
{
    Q_OBJECT

public:
    //static const int sampleCount = 2000;

    AudioFileStream(QXYSeries* series, QObject* parent = nullptr);
    bool init(const QAudioFormat& format);

    enum State { Playing, Paused, Stopped };

    bool loadFile(const QString& filePath);

    bool play(const QString& filePath);
    void pause(); // test
    void stop();

    bool atEnd() const override;

    QFile* getFile();
    QAudioFormat* getFormat();
    State getState();
    void setSampleCount(int sampleCount);
    void setFormat(const QAudioFormat& format);

protected:
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;

private:
    QFile* m_file;
    QBuffer m_input;
    QBuffer m_output;
    QByteArray m_data;
    QAudioDecoder m_decoder;
    QAudioFormat* m_format;

    QXYSeries* m_series;
    QVector<QPointF> m_buffer;

    State m_state;

    bool isInited;
    bool isDecodingFinished;
    int m_sampleCount;

    void clear();

private slots:
    void bufferReady();
    void finished();

signals:
    void stateChanged(AudioFileStream::State state);
    void newData(const QByteArray& data);
};
