#include "AudioFileStream.h"

#include <iostream>

AudioFileStream::AudioFileStream(QXYSeries* series, QObject* parent) :
    QIODevice(parent),
    m_series(series),
    m_sampleCount(0),
    m_input(&m_data),
    m_output(&m_data),
    m_state(State::Stopped),
    m_peakVal(0),
    m_file(new QFile(this))
{
    setOpenMode(QIODevice::ReadOnly);

    isInited = false;
    isDecodingFinished = false;
}

bool AudioFileStream::init(const QAudioFormat& format)
{
    m_format = format;
    m_decoder.setAudioFormat(m_format);

    if (m_decoder.error() != QAudioDecoder::Error::NoError)
    {
        qDebug("AudioFileStream::init() Decoder failed to set audio format.");
        return false;
    }

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

bool AudioFileStream::setFormat(const QAudioFormat& format)
{
    m_decoder.setAudioFormat(format);

    if (m_decoder.error() != QAudioDecoder::Error::NoError)
    {
        m_decoder.setAudioFormat(m_format);
        return false;
    }

    m_format = format;
    return true;
}

QFile* AudioFileStream::getFile()
{
    return m_file;
}

QAudioFormat AudioFileStream::getFormat()
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

void AudioFileStream::drawChartSamples(int start, char* data)
{
    switch (m_format.sampleType())
    {
    case QAudioFormat::Float:
    case QAudioFormat::SignedInt:
        // For 32bit sample size
        if (m_format.sampleSize() == 32)
        {
            int* data_int = (int*)data;
            for (int s = start; s < m_sampleCount; ++s, ++data_int)
            {
                m_buffer[s].setY((int)*data_int / m_peakVal);
            }
        }
        // For 16bit sample size
        else if (m_format.sampleSize() == 16)
        {
            short* data_short = (short*)data;
            for (int s = start; s < m_sampleCount; ++s, ++data_short)
            {
                m_buffer[s].setY((short)*data_short / m_peakVal);
            }
        }
        // For 8bit sample size
        else if (m_format.sampleSize() == 8)
        {
            for (int s = start; s < m_sampleCount; ++s, ++data)
            {
                m_buffer[s].setY((char)*data / m_peakVal);
            }
        }
        break;
    case QAudioFormat::UnSignedInt:
        // For 32bit sample size
        if (m_format.sampleSize() == 32)
        {
            uint* data_int = (uint*)data;
            for (int s = start; s < m_sampleCount; ++s, ++data_int)
            {
                m_buffer[s].setY((uint)*data_int / m_peakVal);
            }
        }
        // For 16bit sample size
        else if (m_format.sampleSize() == 16)
        {
            ushort* data_short = (ushort*)data;
            for (int s = start; s < m_sampleCount; ++s, ++data_short)
            {
                m_buffer[s].setY((ushort)*data_short / m_peakVal);
            }
        }
        // For 8bit sample size
        else if (m_format.sampleSize() == 8)
        {
            for (int s = start; s < m_sampleCount; ++s, ++data)
            {
                m_buffer[s].setY((uchar)*data / m_peakVal);
            }
        }
        break;
    default:
        break;
    }
}

// AudioOutput device (like speaker) calls this function to get new audio data
qint64 AudioFileStream::readData(char* data, qint64 maxSize)
{
    memset(data, 0, maxSize);

    // If playing, read audio from m_output, else don't process any data
    if (m_state == State::Playing)
    {
        static const int resolution = m_format.sampleSize() / 8;

        m_output.read(data, maxSize);

        if (m_buffer.isEmpty()) 
        {
            m_buffer.reserve(m_sampleCount);
            for (int i = 0; i < m_sampleCount; ++i)
                m_buffer.append(QPointF(i, 0));
        }

        // Draw the available sample points to the chart
        int start = 0;
        const int availableSamples = int(maxSize) / resolution;
        if (availableSamples < m_sampleCount)
        {
            start = m_sampleCount - availableSamples;
            for (int s = 0; s < start; ++s)
            {
                m_buffer[s].setY(m_buffer.at(s + availableSamples).y());
            }
        }

        // Draw the rest of the chart's y-values based on sample-size data type
        drawChartSamples(start, data);

        m_series->replace(m_buffer);

        // Send read audio data via signal to output device.
        if (maxSize > 0)
        {
            QByteArray buff(data, maxSize);
            emit newData(buff);
        }

        // If at end of file
        if (m_output.atEnd())
        {
            stop();
        }
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
    if (!clear())
        return false;

    m_decoder.setSourceFilename(filePath);
    m_decoder.start();

    return true;
}

// Start playing the audio file
bool AudioFileStream::play(const QString& filePath)
{
    if (m_decoder.error() != QAudioDecoder::Error::NoError)
    {
        qDebug() << "AudioFileStream::play() ERROR: " << m_decoder.error();
        qDebug() << "AudioFileStream::play() Current value of m_format = " << m_format;
        return false;
    }

    if (m_state == State::Paused)
    {
        qDebug() << "AudioFileStream::play() Resuming audio " << filePath.toLatin1();

        m_state = State::Playing;
        emit stateChanged(m_state);
        return true;
    }
//    else if (m_decoder.state() != QAudioDecoder::State::DecodingState && !isDecodingFinished && m_state == State::Stopped)
//    {
//        qDebug() << "starting decoder";
//        m_decoder.start();
//    }

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

bool AudioFileStream::clear()
{
    m_decoder.stop();
    m_data.clear();
    m_buffer.clear();
    m_series->clear();

    m_output.close();
    m_input.close();

    if (!m_output.open(QIODevice::ReadOnly) || !m_input.open(QIODevice::WriteOnly))
    {
        return false;
    }

    isDecodingFinished = false;

    return true;
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

    if (m_peakVal == qreal(0))
    {
        m_peakVal = getPeakValue(buffer.format());
    }

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

qreal AudioFileStream::getPeakValue(const QAudioFormat& format)
{
    // Note: Only the most common sample formats are supported
    if (!format.isValid())
        return qreal(0);

    if (format.codec() != "audio/pcm")
        return qreal(0);

    switch (format.sampleType()) {
        case QAudioFormat::Unknown:
            break;
        case QAudioFormat::Float:
            if (format.sampleSize() != 32) // other sample formats are not supported
                return qreal(0);
            return qreal(1.00003);
        case QAudioFormat::SignedInt:
            if (format.sampleSize() == 32)
            {
            #ifdef Q_OS_WIN
                return qreal(INT_MAX);
            #endif
            #ifdef Q_OS_UNIX
                return qreal(SHRT_MAX);
            #endif
            }
            if (format.sampleSize() == 16)
            {
                return qreal(SHRT_MAX);
            }
            if (format.sampleSize() == 8)
            {
                return qreal(CHAR_MAX);
            }
            break;
        case QAudioFormat::UnSignedInt:
            if (format.sampleSize() == 32)
            {
                return qreal(UINT_MAX);
            }
            if (format.sampleSize() == 16)
            {
                return qreal(USHRT_MAX);
            }
            if (format.sampleSize() == 8)
            {
                return qreal(UCHAR_MAX);
            }
            break;
    }

    return qreal(0);
}
