#include "FFTWorkerThread.h"

#include <math.h>

FFTWorkerThread::FFTWorkerThread()
    : m_dataBuffer(nullptr)
{
    qRegisterMetaType<QVector<QPointF>>("QVector<QPointF>");
}

FFTWorkerThread::~FFTWorkerThread()
{

}

void FFTWorkerThread::setDataBuffer(const QBuffer* dataBuffer)
{
    m_dataBuffer = dataBuffer;
}

void FFTWorkerThread::setAudioFormat(QAudioFormat format)
{
    m_format = format;
}

void FFTWorkerThread::clearData()
{
    m_spectrumBuffer.clear();
}

// Common implementation of the Cooley-Tukey FFT algorithm, modified for our use case.
// https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
std::vector<std::pair<size_t, double>> FFTWorkerThread::cooleyTukey(std::vector<double> &real, std::vector<double> &imag)
{
    std::vector<std::pair<size_t, double>> output;

    // Check the length of both real/imag vectors
    size_t n = real.size();
    size_t test = n;
    if (n != imag.size())
        throw std::invalid_argument("Mismatched lengths");

    // Compute the number of total levels for the FFT using bit shifting
    int levels = 0;
    for (size_t temp = n; temp > 1U; temp >>= 1)
        levels++;

    // If length is not a power of 2, resize both vectors to next highest power of 2.
    if (static_cast<size_t>(1U) << levels != n)
    {
        int powOf2 = static_cast<size_t>(1U) << (levels + 1);
        real.resize(powOf2);
        imag.resize(powOf2);
        n = powOf2;
        levels++;
        qDebug() << "FFTWorkerThread::cooleyTukey() padded vector to size " << powOf2;
    }

    output.reserve(n);

    // cos/sin calculations
    const ulong samplesPerSec = m_format.bytesForDuration(1e6) / (m_format.sampleSize() / 8);
    std::vector<double> cosTable(n / 2);
    std::vector<double> sinTable(n / 2);
    for (size_t i = 0; i < n / 2; i++)
    {
        if (isInterruptionRequested())
        {
            clearData();
            return output;
        }

        cosTable[i] = std::cos(2 * M_PI * i / n);
        sinTable[i] = std::sin(2 * M_PI * i / n);
    }

    // Bit-reversed addressing permutation
    // https://en.wikipedia.org/wiki/Bit-reversal_permutation
    for (size_t i = 0; i < n; i++)
    {
        if (isInterruptionRequested())
        {
            clearData();
            return output;
        }

        // Reverse the current index with bit width equal to number of levels
        size_t j = FFTUtils::reverseBits(i, levels);

        // If the reversed index is greater than the current index,
        // swap the values in the real/imaginary vectors.
        if (j > i)
        {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    // Cooley-Tukey decimation-in-time radix-2 FFT algorithm
    for (size_t size = 2; size <= n; size *= 2)
    {
        size_t halfsize = size / 2;
        size_t tablestep = n / size;
        for (size_t i = 0; i < n; i += size)
        {
            for (size_t j = i, k = 0; j < i + halfsize; j++, k += tablestep)
            {
                if (isInterruptionRequested())
                {
                    clearData();
                    return output;
                }

                size_t l = j + halfsize;
                double tpre =  real[l] * cosTable[k] + imag[l] * sinTable[k];
                double tpim = -real[l] * sinTable[k] + imag[l] * cosTable[k];
                real[l] = real[j] - tpre;
                imag[l] = imag[j] - tpim;
                real[j] += tpre;
                imag[j] += tpim;
            }
        }
        if (size == n)  // Prevent overflow when calculating size *= 2
            break;
    }

    double maxSum = 0.0;

    // Fill output vector
    int prevK = -1;
    for (size_t i = 0; i < real.size(); ++i)
    {
        // Calculate magnitude of complex element
        std::complex<double> complex(real[i], imag[i]);
        double abs = std::abs(complex);

        // Update maxSum
        if (abs > maxSum)
            maxSum = abs;

        // Get the corresponding frequency bin from the current index.
        int k = FFTUtils::index2Freq(i, samplesPerSec, real.size());

        // Skip duplicate frequencies (since we are casting to int, we lose the float precision)
        if (k == prevK)
            continue;

        // Add the frequency, amplitude pair to the output vector
        output.push_back(std::make_pair(k, abs));
        prevK = k;
    }

    // Normalization by maxSum
    for (size_t i = 0; i < output.size(); ++i)
    {
        output[i].second /= maxSum;
    }

    return output;
}

void FFTWorkerThread::run()
{
    m_spectrumBuffer.clear();

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    if (N == 0)
    {
        return;
    }

    qDebug() << "FFTWorkerThread::run() Number of samples received: " << N;

    // Get raw data and define K
    const char* data = m_dataBuffer->buffer().constData();
    short* data_short = (short*)data;
    const int K = Constants::MAX_FREQUENCY;

    m_spectrumBuffer.reserve(K - Constants::MIN_FREQUENCY);

    std::vector<std::pair<size_t, double>> output;

    // Prepare real/imag vectors
    std::vector<double> real(data_short, data_short + N);
    std::vector<double> imag;
    imag.resize(real.size());

    // Exception handling, should never get inside catch.
    try {
        output = cooleyTukey(real, imag);
    }  catch (std::invalid_argument e) {
        qDebug() << "Invalid sizes of reals/imags vectors, aborting FFTWorkerThread::run()";
        return;
    }

    if (isInterruptionRequested())
    {
        clearData();
        return;
    }

    // Fill the spectrum graph buffer
    for (size_t i = 0; i < output.size(); ++i)
    {
        if (output[i].first > Constants::MAX_FREQUENCY)
            break;
        else if (output[i].first < Constants::MIN_FREQUENCY)
            continue;

        QPointF point(output[i].first, output[i].second);
        m_spectrumBuffer.append(point);
    }

    emit resultReady(m_spectrumBuffer);
}
