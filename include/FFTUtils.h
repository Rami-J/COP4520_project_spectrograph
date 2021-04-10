#ifndef FFTUTILS_H
#define FFTUTILS_H

#include <cstddef>

class FFTUtils
{
public:
    // Reverses bits of parameter val with bit-width width.
    static size_t reverseBits(size_t val, int width);

    // Returns the corresponding frequency, given an index, sample rate, and number
    // of elements in the FFT output vector.
    static double index2Freq(int i, double samples, int nFFT);

};

#endif // FFTUTILS_H
