#ifndef CURVELINES_H
#define CURVELINES_H

#include <cmath>
#include <float.h>
#include <QVector2D>
#include <QRectF>
#include <QVector>
#include <QObject>

class CurvePoint
{
public:
    enum PointType{
        Default = 0x00,
        Line = 0x01,
        Curve = 0x02,
    };

public:
    CurvePoint(PointType t = Default) :
        type(t), drag(false), touch(false), pos(0, 0),
        drag2(false), touch2(false), pos2(0, 0) {}

    CurvePoint(float x, float y, PointType t = Default) :
        type(t), drag(false), touch(false), pos(x, y),
        drag2(false), touch2(false), pos2(x, y) {}

    CurvePoint(const QVector2D &p, PointType t = Default) :
        type(t), drag(false), touch(false), pos(p),
        drag2(false), touch2(false), pos2(p) {}

public:
    CurvePoint& operator=(const CurvePoint& p)
    {
        type = p.type;
        pos = p.pos;
        pos2 = p.pos2;
        return *this;
    }

    bool operator >(const CurvePoint& p)
    {
        return pos.x() > p.pos.x();
    }

    bool operator <(const CurvePoint& p)
    {
        return pos.x() < p.pos.x();
    }

    bool operator ==(const CurvePoint& p)
    {
        return abs(pos.x() - p.pos.x()) <= 0.01f;
    }

public:
    PointType type;
    bool drag;
    bool touch;
    QVector2D pos;
    bool drag2;
    bool touch2;
    QVector2D pos2;
};

class CurveLines : public QObject
{
    Q_OBJECT
public:
    enum MoveType{
        X_Axis = 0x00,
        Y_Axis = 0x01,
        XY_Axis = 0x02,
    };

    enum TouchType{
        Touch_Move = 0x00,
        Touch_Add = 0x01,
        Touch_Take = 0x02,
    };

public:
    CurveLines();
    ~CurveLines();

signals:
    void updateCurve(const QVector<CurvePoint> &data);

public slots:
    void onCurve(const QVector<CurvePoint>& points);

public:
    int pointsSize();
    int pointsTouchSize();
    int pointsDragSize();

    void insertPoint(const CurvePoint& point);
    void selectPoints();
    void releasePoints();
    void updatePoints();

public:
    float getValue(float x);
    float getMinValue();
    float getMaxValue();
    float getAverageValue();

public:
    int touchPoints(const QRectF& rect);
    int touchPoints(const QVector2D& pos, float scale);

    void findTouchPoint(const QVector2D& pos);
    void leftTouchPoint(TouchType type);
    void rightTouchPoint(TouchType type);

    int deleteTouchPoint();
    int ceilTouchPoint(MoveType type);
    int floorTouchPoint(MoveType type);

    int moveTouchPoint(const QVector2D& offset, MoveType type);
    int moveDragPoint(const QVector2D& offset, MoveType type);

public:
    CurvePoint& firstPoint();
    CurvePoint& lastPoint();

    CurvePoint& currentPoint(int i);
    CurvePoint& evaluatePoint(int i);

    QVector2D evaluate(int i, float t);
    QVector2D evaluate(float t, const CurvePoint& point, const CurvePoint& pointd);

private:
    float m_min;
    float m_max;
    float m_average;
    QVector<CurvePoint> m_points;
};

#endif // CURVELINES_H
