#include "FTAnalysis.h"

#include <QtCore>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FTAnalysis ftAnalysis(&a);

    // This will cause the application to exit when
    // the FTAnalysis signals finish.
    QObject::connect(&ftAnalysis, &FTAnalysis::finished, &a, &QCoreApplication::quit);

    // This will run the performance analysis task from the application event loop.
    QTimer::singleShot(0, &ftAnalysis, &FTAnalysis::startPerformanceAnalysis);

    return a.exec();
}
