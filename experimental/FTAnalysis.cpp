#include "FTAnalysis.h"

static const QString SAMPLE_FILE_1s = ":/audio/440Hz-1s.wav";
static const QString SAMPLE_FILE_3s = ":/audio/440Hz-3s.wav";
static const QString SAMPLE_FILE_30s = ":/audio/440Hz-30s.wav";
static const int NUM_TRIALS = 10;

FTAnalysis::FTAnalysis(QObject *parent)
    : QObject(parent),
      m_avgTime(0.0)
{
    connect(&m_ftController, &FTController::spectrumDataReady, this, &FTAnalysis::receiveFTResults);
    connect(&m_decoder, &QAudioDecoder::bufferReady, this, &FTAnalysis::writeAudioDataToBuffer);
}

void FTAnalysis::startPerformanceAnalysis()
{
    std::cout << "Conducting performance analysis on parallel/sequential FFT for " << NUM_TRIALS << " trials each." << std::endl;

    std::cout << "Starting Single-Threaded FFT on 440Hz-1s.wav" << std::endl;
    startTrials(SAMPLE_FILE_1s, SLOT(calcFFT()));
    std::cout << std::endl;

    std::cout << "Starting Single-Threaded FFT on 440Hz-3s.wav" << std::endl;
    startTrials(SAMPLE_FILE_3s, SLOT(calcFFT()));
    std::cout << std::endl;

    std::cout << "Starting Single-Threaded FFT on 440Hz-30s.wav" << std::endl;
    startTrials(SAMPLE_FILE_30s, SLOT(calcFFT()));
    std::cout << std::endl;

    std::cout << "Starting Multi-Threaded FFT on 440Hz-1s.wav" << std::endl;
    startTrials(SAMPLE_FILE_1s, SLOT(calcDistributedFFT()));
    std::cout << std::endl;

    std::cout << "Starting Multi-Threaded FFT on 440Hz-3s.wav" << std::endl;
    startTrials(SAMPLE_FILE_3s, SLOT(calcDistributedFFT()));
    std::cout << std::endl;

    std::cout << "Starting Multi-Threaded FFT on 440Hz-30s.wav" << std::endl;
    startTrials(SAMPLE_FILE_30s, SLOT(calcDistributedFFT()));
    std::cout << std::endl;

    emit finished();
}

void FTAnalysis::startTrials(const QString& filePath, const char* slot)
{
    connect(&m_decoder, SIGNAL(finished()), this, slot);
    for (int i = 0; i < NUM_TRIALS; ++i)
    {
        startDecoder(filePath);
    }
    disconnect(&m_decoder, SIGNAL(finished()), this, slot);

    m_avgTime /= NUM_TRIALS;

    std::cout << "Average running time: " << m_avgTime << std::endl;

    m_avgTime = 0.0;
}

void FTAnalysis::receiveFTResults(const QVector<QPointF> points, const double elapsedSeconds)
{
    Q_UNUSED(points);

    m_avgTime += elapsedSeconds;

    //std::cout << "Elapsed time: " << elapsedSeconds << "s" << std::endl;
}

void FTAnalysis::writeAudioDataToBuffer()
{
    const QAudioBuffer& buffer = m_decoder.read();

    m_format = buffer.format();

    const int length = buffer.byteCount();
    const char* data = buffer.constData<char>();
    m_ftController.getDataBuffer()->write(data, length);
}

void FTAnalysis::calcFFT()
{
    m_ftController.startFFTInAThread(m_format);

    QSignalSpy spy(&m_ftController, &FTController::spectrumDataReady);
    spy.wait();
}

void FTAnalysis::calcDistributedFFT()
{
    m_ftController.startDistributedFFT(m_format);

    QSignalSpy spy(&m_ftController, &FTController::spectrumDataReady);
    spy.wait();
}

void FTAnalysis::startDecoder(const QString& filePath)
{
    m_ftController.clear();

    if (m_file.isOpen())
        m_file.close();

    m_file.setFileName(filePath);
    m_file.open(QIODevice::ReadOnly);

    m_decoder.setSourceDevice(&m_file);
    m_decoder.start();

    QSignalSpy spy(&m_decoder, &QAudioDecoder::finished);
    spy.wait();
}
