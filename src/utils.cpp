#include <QtMultimedia/QAudioFormat>
#include "utils.h"

qint64 Utils::audioDuration(const QAudioFormat& format, qint64 bytes)
{
    return (bytes * 1000000) /
        (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8));
}

qint64 Utils::audioLength(const QAudioFormat& format, qint64 microSeconds)
{
    qint64 result = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
        * microSeconds / 1000000;
    result -= result % (format.channelCount() * format.sampleSize());
    return result;
}

qreal Utils::nyquistFrequency(const QAudioFormat& format)
{
    return format.sampleRate() / 2;
}

QString Utils::formatToString(const QAudioFormat& format)
{
    QString result;

    if (QAudioFormat() != format) {
        if (format.codec() == "audio/pcm") {
            Q_ASSERT(format.sampleType() == QAudioFormat::SignedInt);

            const QString formatEndian = (format.byteOrder() == QAudioFormat::LittleEndian)
                ? QString("LE") : QString("BE");

            QString formatType;
            switch (format.sampleType()) {
            case QAudioFormat::SignedInt:
                formatType = "signed";
                break;
            case QAudioFormat::UnSignedInt:
                formatType = "unsigned";
                break;
            case QAudioFormat::Float:
                formatType = "float";
                break;
            case QAudioFormat::Unknown:
                formatType = "unknown";
                break;
            }

            QString formatChannels = QString("%1 channels").arg(format.channelCount());
            switch (format.channelCount()) {
            case 1:
                formatChannels = "mono";
                break;
            case 2:
                formatChannels = "stereo";
                break;
            }

            result = QString("%1 Hz %2 bit %3 %4 %5")
                .arg(format.sampleRate())
                .arg(format.sampleSize())
                .arg(formatType)
                .arg(formatEndian)
                .arg(formatChannels);
        }
        else {
            result = format.codec();
        }
    }

    return result;
}

bool Utils::isPCM(const QAudioFormat& format)
{
    return (format.codec() == "audio/pcm");
}


bool Utils::isPCMS16LE(const QAudioFormat& format)
{
    return isPCM(format) &&
        format.sampleType() == QAudioFormat::SignedInt &&
        format.sampleSize() == 16 &&
        format.byteOrder() == QAudioFormat::LittleEndian;
}

const qint16  PCMS16MaxValue = 32767;
const quint16 PCMS16MaxAmplitude = 32768; // because minimum is -32768

qreal Utils::pcmToReal(qint16 pcm)
{
    return qreal(pcm) / PCMS16MaxAmplitude;
}

qint16 Utils::realToPcm(qreal real)
{
    return real * PCMS16MaxValue;
}
