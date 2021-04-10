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

size_t FFTWorkerThread::reverseBits(size_t val, int width)
{
    size_t result = 0;

    // Loop through each bit in val, bitwise-or the LSB with the current reversed result
    // and right-shift out the LSB each iteration.
    for (int i = 0; i < width; i++, val >>= 1)
    {
        result = (result << 1) | (val & 1U);
    }

    return result;
}

double index2Freq(int i, double samples, int nFFT) 
{
    return (double)i * (samples / nFFT);
}

// TODO: return vector of frequency, value pairs
// TODO: maybe instead also do normalization of values and return the QVector<QPointF> directly?
std::vector<std::pair<size_t, double>> FFTWorkerThread::cooleyTukey(std::vector<double> &real, std::vector<double> &imag)
{
    std::vector<std::pair<size_t, double>> output;

    // Check the length of both real/imag vectors
    size_t n = real.size();
    size_t test = n;
    qDebug() << "n = " << n;
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
        qDebug() << "Padded vector to size " << powOf2 << " levels = " << levels;
    }

    output.reserve(n);

    // cos/sin calculations
    const ulong samplesPerSec = m_format.bytesForDuration(1e6) / (m_format.sampleSize() / 8);
    std::vector<double> cosTable(n / 2);
    std::vector<double> sinTable(n / 2);
    for (size_t i = 0; i < n / 2; i++)
    {
        cosTable[i] = std::cos(2 * M_PI * i / n);
        sinTable[i] = std::sin(2 * M_PI * i / n);
    }

    // Bit-reversed addressing permutation
    // https://en.wikipedia.org/wiki/Bit-reversal_permutation
    for (size_t i = 0; i < n; i++)
    {
        // Reverse the current index with bit width equal to number of levels
        size_t j = reverseBits(i, levels);

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
    int prevIndex = -1;
    for (size_t i = 0; i < real.size(); ++i)
    {
        std::complex<double> complex(real[i], imag[i]);
        double abs = std::abs(complex);

        if (abs > maxSum)
            maxSum = abs;

        int index = index2Freq(i, samplesPerSec, real.size());

        if (index == prevIndex)
            continue;

        output.push_back(std::make_pair(index, abs));
        prevIndex = index;
        //output.push_back(std::make_pair(i, real.size()), abs));
    }

    // Normalization by maxSum
    for (size_t i = 0; i < output.size(); ++i)
    {
        output[i].second /= maxSum;
    }

    qDebug() << "output size = " << output.size();

    return output;
}

void FFTWorkerThread::run()
{
    m_spectrumBuffer.clear();

    // Calculate number of samples
    const ulong N = m_dataBuffer->bytesAvailable() / (m_format.sampleSize() / 8);

    qDebug() << "N = " << N;

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
    output.reserve(N);

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
