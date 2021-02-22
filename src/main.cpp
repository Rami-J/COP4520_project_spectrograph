#include "SpectrographUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SpectrographUI w;
    w.show();
    return a.exec();
}
