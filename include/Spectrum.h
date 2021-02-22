#ifndef SPECTRUM_H
#define SPECTRUM_H

// TODO: implement our own fast fourier transform algorithm (single-threaded first, then incorporate multi-threads)

#include <qglobal.h>
#include "utils.h"
//#include "fftreal_wrapper.h" // For FFTLengthPowerOfTwo

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// Number of audio samples used to calculate the frequency spectrum
static const int FFTLengthPowerOfTwo = 12;
const int    SpectrumLengthSamples = Utils::PowerOfTwo<FFTLengthPowerOfTwo>::Result;

// Number of bands in the frequency spectrum
const int    SpectrumNumBands = 10;

// Lower bound of first band in the spectrum
const qreal  SpectrumLowFreq = 0.0; // Hz

// Upper band of last band in the spectrum
const qreal  SpectrumHighFreq = 1000.0; // Hz

// Waveform window size in microseconds
const qint64 WaveformWindowDuration = 500 * 1000;

// Length of waveform tiles in bytes
// Ideally, these would match the QAudio*::bufferSize(), but that isn't
// available until some time after QAudio*::start() has been called, and we
// need this value in order to initialize the waveform display.
// We therefore just choose a sensible value.
const int   WaveformTileLength = 4096;

// Fudge factor used to calculate the spectrum bar heights
const qreal SpectrumAnalyserMultiplier = 0.15;

// Disable message timeout
const int   NullMessageTimeout = -1;


//-----------------------------------------------------------------------------
// Types and data structures
//-----------------------------------------------------------------------------

enum WindowFunction {
    NoWindow,
    HannWindow
};

const WindowFunction DefaultWindowFunction = HannWindow;

struct Tone
{
    Tone(qreal freq = 0.0, qreal amp = 0.0)
        : frequency(freq), amplitude(amp)
    { }

    // Start and end frequencies for swept tone generation
    qreal   frequency;

    // Amplitude in range [0.0, 1.0]
    qreal   amplitude;
};

struct SweptTone
{
    SweptTone(qreal start = 0.0, qreal end = 0.0, qreal amp = 0.0)
        : startFreq(start), endFreq(end), amplitude(amp)
    {
        Q_ASSERT(end >= start);
    }

    SweptTone(const Tone& tone)
        : startFreq(tone.frequency), endFreq(tone.frequency), amplitude(tone.amplitude)
    { }

    // Start and end frequencies for swept tone generation
    qreal   startFreq;
    qreal   endFreq;

    // Amplitude in range [0.0, 1.0]
    qreal   amplitude;
};

// Handle some dependencies between macros defined in the .pro file

#ifdef DISABLE_WAVEFORM
#undef SUPERIMPOSE_PROGRESS_ON_WAVEFORM
#endif

#endif // SPECTRUM_H

