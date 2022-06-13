#include "qcurveeditwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCurveEditWidget w;
    w.show();

    return a.exec();
}
