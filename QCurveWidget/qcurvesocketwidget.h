#ifndef QCURVESOCKETWIDGET_H
#define QCURVESOCKETWIDGET_H

#include <QWidget>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "curvelines.h"

class QCurveCenterData : public QObject
{
    Q_OBJECT
public:
    static QCurveCenterData *instance()
    {
        static QCurveCenterData self;
        return &self;
    }

public:
    explicit QCurveCenterData(QObject *parent = nullptr);

signals:
    void updateCurve(const QVector<CurvePoint> &data);

public slots:
    void onZoom(float scale, QPoint offset, QRect rect);
    void onCurve(const QVector<CurvePoint>& points);
    void onSocket(const QString& data);

public:
    int remoteState();
    void remoteConnect();
    void remoteDisconnect();
    void remoteSend();

private:
    int m_remote;
    QJsonObject    m_msgZoom;
    QJsonArray      m_msgPoints;
    QWebSocket  *m_webSocket;
};

#endif // QCURVESOCKETWIDGET_H
