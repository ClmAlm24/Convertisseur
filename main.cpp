#include "conversion.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Conversion w;
    w.showMaximized();
    return a.exec();
}
