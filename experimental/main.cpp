#include "FTAnalysis.h"

#include <QVector>
#include <QPointF>
#include <QtCore>
#include <QCoreApplication>

// TODO: decode sample audio files in the experimental/ directory using
// QAudioDecoder and pass the data into all 4 transform algorithms. Time each
// function using std::chrono over a set number of trials.
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FTAnalysis ftAnalysis(&a);

    // This will cause the application to exit when
    // the FTAnalysis signals finished.
    QObject::connect(&ftAnalysis, &FTAnalysis::finished, &a, &QCoreApplication::quit);

    // This will run the performance analysis task from the application event loop.
    QTimer::singleShot(0, &ftAnalysis, &FTAnalysis::startPerformanceAnalysis);

    return a.exec();
}
