#include "AudioFileStream.h"

#include <iostream>

AudioFileStream::AudioFileStream(QXYSeries* series, QObject* parent) :
    QIODevice(parent),
    m_series(series),
    m_sampleCount(0),
    m_input(&m_data),
    m_output(&m_data),
    m_state(State::Stopped),
    m_file(new QFile(this)),
    m_format(new QAudioFormat())
{
    setOpenMode(QIODevice::ReadOnly);

    isInited = false;
    isDecodingFinished = false;
}

bool AudioFileStream::init(const QAudioFormat& format)
{
    m_format = new QAudioFormat(format);
    m_decoder.setAudioFormat(*m_format);

    connect(&m_decoder, &QAudioDecoder::bufferReady, this, &AudioFileStream::bufferReady);
    connect(&m_decoder, &QAudioDecoder::finished, this, &AudioFileStream::finished);

    // Initialize buffers
    if (!m_output.open(QIODevice::ReadOnly) || !m_input.open(QIODevice::WriteOnly))
    {
        return false;
    }

    isInited = true;

    return true;
}

void AudioFileStream::setFormat(const QAudioFormat& format)
{
    delete m_format;
    m_format = new QAudioFormat(format);
    m_decoder.setAudioFormat(*m_format);
}

QFile* AudioFileStream::getFile()
{
    return m_file;
}

QAudioFormat* AudioFileStream::getFormat()
{
    return m_format;
}

AudioFileStream::State AudioFileStream::getState()
{
    return m_state;
}

void AudioFileStream::setSampleCount(int sampleCount)
{
    m_sampleCount = sampleCount;
}

// AudioOutput device (like speaker) calls this function to get new audio data
qint64 AudioFileStream::readData(char* data, qint64 maxSize)
{
    static const int resolution = m_format->sampleSize() / 8;

    memset(data, 0, maxSize);

    // If playing, read audio from m_output, else don't process any data
    if (m_state == State::Playing)
    {
        m_output.read(data, maxSize);

        if (m_buffer.isEmpty()) {
            m_buffer.reserve(m_sampleCount);
            for (int i = 0; i < m_sampleCount; ++i)
                m_buffer.append(QPointF(i, 0));
        }

        int start = 0;
        const int availableSamples = int(maxSize) / resolution;
        if (availableSamples < m_sampleCount) {
            start = m_sampleCount - availableSamples;
            for (int s = 0; s < start; ++s) {
                m_buffer[s].setY(m_buffer.at(s + availableSamples).y());
            }
        }

        for (int s = start; s < m_sampleCount; ++s, data += resolution) {
            //m_buffer[s].setY(qreal(uchar(*data) - 128) / qreal(128));
            m_buffer[s].setY(qreal(*data));
        }
            
        m_series->replace(m_buffer);
        
        // There is we send readed audio data via signal, for ability get audio signal for listeners of this signal.
        // This emulates QAudioProbe behaviour for retrieve audio data which of sent to output device (speaker).
        if (maxSize > 0)
        {
            //QByteArray buff(data, maxSize);
            //QByteArray buff(data, (m_sampleCount - start) * resolution);
            //emit newData(buff);
        }
    }

    // If at end of file and not already stopped, stop
    if (m_state != State::Stopped && m_output.atEnd())
    {
        stop();
    }

    return maxSize;
}

qint64 AudioFileStream::writeData(const char* data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    return 0;
}

bool AudioFileStream::loadFile(const QString& filePath)
{
    clear();

    m_decoder.setSourceFilename(filePath);
    m_decoder.start();

    return true;
}

// Start playing the audio file
bool AudioFileStream::play(const QString& filePath)
{
    if (m_state == State::Paused)
    {
        qDebug() << "resuming audio " << filePath.toLatin1();

        m_state = State::Playing;
        emit stateChanged(m_state);
        return true;
    }
    else if (m_state == State::Stopped)
    {
        m_decoder.start();
    }

    m_state = State::Playing;
    emit stateChanged(m_state);

    return true;
}

// Pause the currently playing audio file
void AudioFileStream::pause()
{
    m_state = State::Paused;
    emit stateChanged(m_state);
}

// Stop playing audio file
void AudioFileStream::stop()
{
    m_file->close();
    clear();
    m_state = State::Stopped;
    emit stateChanged(m_state);
}

void AudioFileStream::clear()
{
    m_decoder.stop();
    m_data.clear();

    m_output.close();
    m_input.close();

    if (!m_output.open(QIODevice::ReadOnly) || !m_input.open(QIODevice::WriteOnly))
    {
        return;
    }

    isDecodingFinished = false;
}

// Determines if reached the end of audio file
bool AudioFileStream::atEnd() const
{
    return m_output.size()
        && m_output.atEnd()
        && isDecodingFinished;
}

// QAudioDecoder Logic 
// This method is responsible for decoding the audio file and wrtiing audio data to stream buffer
// Only runs when decoder processed some audio data
void AudioFileStream::bufferReady() // SLOT
{
    const QAudioBuffer& buffer = m_decoder.read();

    const int length = buffer.byteCount();

    const char* data = buffer.constData<char>();

    // For debugging start/endtime
    /*const qreal s_per_us = (1 / 1e6);
    qint64 duration = buffer.duration() * s_per_us;
    qint64 startTime = buffer.startTime() * s_per_us;

    qDebug() << duration << " " << startTime << " " << buffer.isValid();*/
    //qDebug() << m_format->sampleSize();

    m_input.write(data, length);
}

// Runs when decoder finished decoding
void AudioFileStream::finished() // SLOT
{
    isDecodingFinished = true;
}
