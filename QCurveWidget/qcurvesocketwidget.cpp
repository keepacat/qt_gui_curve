#include "qcurvesocketwidget.h"

QCurveCenterData::QCurveCenterData(QObject *parent) : QObject(parent), m_remote(0)
{
    m_webSocket = new QWebSocket();

    QObject::connect(m_webSocket, &QWebSocket::connected, [&](){
        qDebug() << "connected";
        m_remote = 1;
    });

    QObject::connect(m_webSocket, &QWebSocket::disconnected, [&](){
        qDebug() << "disconnected";
        m_remote = 0;
    });

    QObject::connect(m_webSocket, &QWebSocket::binaryFrameReceived, [&](const QByteArray &msg){
        qDebug() << msg;
        onSocket(msg);
    });
}

void QCurveCenterData::onCurve(const QVector<CurvePoint> &points)
{
    QJsonArray socketData;
    for(const CurvePoint& point : points)
    {
        QJsonObject object;
        object["type"] = point.type;
        QJsonArray array;
        array.append((double)point.pos.x());
        array.append((double)point.pos.y());
        object["pos"] = array;
        QJsonArray array2;
        array2.append((double)point.pos2.x());
        array2.append((double)point.pos2.y());
        object["pos2"] = array2;
        socketData.append(object);
    }

    QJsonDocument doc;
    doc.setArray(socketData);
    m_webSocket->sendBinaryMessage(doc.toJson(QJsonDocument::Compact));
}

void QCurveCenterData::onSocket(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray socketData = doc.array();

    QVector<CurvePoint> curveData;
    for(const QJsonValue& item : socketData)
    {
        CurvePoint point;
        QJsonObject object = item.toObject();
        point.type = CurvePoint::PointType(object["type"].toInt());
        QJsonArray array = object["pos"].toArray();
        point.pos.setX((float)array[0].toDouble());
        point.pos.setY((float)array[1].toDouble());
        QJsonArray array2 = object["pos2"].toArray();
        point.pos2.setX((float)array2[0].toDouble());
        point.pos2.setY((float)array2[1].toDouble());
    }
    emit updateCurve(curveData);
}

int QCurveCenterData::remoteState()
{
    return m_remote;
}

void QCurveCenterData::remoteConnect()
{
    m_webSocket->open(QUrl("ws://localhost:80/curve/gui"));
}

void QCurveCenterData::remoteDisconnect()
{
    m_webSocket->close();
}
