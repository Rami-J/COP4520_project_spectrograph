#include "FFTUtils.h"

size_t FFTUtils::reverseBits(size_t val, int width)
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

double FFTUtils::index2Freq(int i, double samples, int nFFT)
{
    return (double)i * (samples / nFFT);
}