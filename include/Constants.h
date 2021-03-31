#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
	static const int MIN_FREQUENCY = 100;
	static const int MAX_FREQUENCY = 1000;

	// Define number of DFT workers to 9 since we can split up the k-frequency calculations evenly
	// because MAX_FREQUENCY - MIN_FREQUENCY = 1000 - 100 = 900 and 900 / 9 = 100 frequency bins per thread
	static const int NUM_DFT_WORKERS = 9;
}

#endif // CONSTANTS_H