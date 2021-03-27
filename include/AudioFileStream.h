#ifndef AUDIOFILESTREAM_H
#define AUDIOFILESTREAM_H

#include "Waveform.h"
#include "Spectrograph.h"

#include <thread>
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
    AudioFileStream(Waveform* waveform, Spectrograph* spectrograph, QObject* parent = nullptr);
    bool init(const QAudioFormat& format);

    enum State { Playing, Paused, Stopped };

    bool loadFile(const QString& filePath);

    bool play(const QString& filePath);
    void pause();
    void stop();

    bool atEnd() const override;

    QFile* getFile();
    QAudioFormat getFormat();
    State getState();
    void setSampleCount(int sampleCount);
    bool setFormat(const QAudioFormat& format);
    static qreal getPeakValue(const QAudioFormat& format);

protected:
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char* data, qint64 len) override;

private:
    void drawChartSamples(int start, char* data);

    QFile* m_file;
    QBuffer m_input;
    QBuffer m_output;
    QByteArray m_data;
    QAudioDecoder m_decoder;
    QAudioFormat m_format;

    Waveform* m_waveform;
    QVector<QPointF> m_waveformBuffer;
    Spectrograph* m_spectrograph;
    QVector<QPointF> m_spectrumBuffer;

    State m_state;
    qreal m_peakVal;

    bool isInited;
    bool isDecodingFinished;

    bool clear();

private slots:
    void bufferReady();
    void finished();

signals:
    void stateChanged(AudioFileStream::State state);
    void newData(const QByteArray& data);
};

#endif // AUDIOFILESTREAM_H