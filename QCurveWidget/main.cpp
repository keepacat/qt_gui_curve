#include <QApplication>
#include "qcurveeditwidget.h"
#include "qcurvesocketwidget.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCurveEditWidget w;
    w.show();

    QCurveCenterData *socket = QCurveCenterData::instance();
    socket->remoteConnect();

    CurveLines *line = w.getCurveLines();
    QObject::connect(socket, &QCurveCenterData::updateCurve, line, &CurveLines::onCurve);
    QObject::connect(line, &CurveLines::updateCurve, socket, &QCurveCenterData::onCurve);
    QObject::connect(&w, &QCurveEditWidget::updateZoom, socket, &QCurveCenterData::onZoom);

    return a.exec();
}
