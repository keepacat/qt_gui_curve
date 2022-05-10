#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCurveWidget w;
    w.show();

    return a.exec();
}
