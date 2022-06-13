#ifndef QCURVEEDITWIDGET_H
#define QCURVEEDITWIDGET_H

#include <cmath>
#include <QWidget>
#include <QVector2D>
#include <QTimer>
#include <QDebug>
#include "float.h"

const int DotSize = 3;
const float MinimumFloatMistake = 0.01f;

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
        return abs(pos.x() - p.pos.x()) <= MinimumFloatMistake;
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

class CurveLines
{
public:
    enum MoveType{
        X_Axis = 0x00,
        Y_Axis = 0x01,
        A_Axis = 0x02,
    };

    enum TouchType{
        Touch_Move = 0x00,
        Touch_Add = 0x01,
        Touch_Take = 0x02,
    };

    CurveLines() : valueMin(0), valueMax(0), valueAverage(0) {}

public:
    int pointsSize()
    {
        return points.size();
    }

    void selectPoints()
    {
        for (int i = 0; i < points.size(); i++)
        {
            points[i].drag = false;
            points[i].touch = true;
            points[i].drag2 = false;
            points[i].touch2 = true;
        }
    }

    void releasePoints()
    {
        for (int i = 0; i < points.size(); i++)
        {
            points[i].drag = false;
            points[i].touch = false;
            points[i].drag2 = false;
            points[i].touch2 = false;
        }
    }

public:
    float getValue(float x)
    {
        for (int i = 0; i < points.size(); i++)
        {
            if ((points[i].pos.x() - x) <= MinimumFloatMistake)
            {
                return points[i].pos.y();
            }
            if (i > 0)
            {
                const CurvePoint& point = currentPoint(i);
                const CurvePoint& pointd = evaluatePoint(i);
                if (point.pos.x() < x && pointd.pos.x() > x)
                {
                    float ox = point.pos.x() - pointd.pos.x();
                    float t = (x - pointd.pos.x()) / ox;
                    return evaluate(i, t).y();
                }
            }
        }
        return 0;
    }

    float getMinValue()
    {
        return valueMin;
    }

    float getMaxValue()
    {
        return valueMax;
    }

    float getAverageValue()
    {
        return valueAverage;
    }

    void updateValue()
    {
        if(points.size())
        {
            float min = FLT_MAX;
            float max = -FLT_MAX;
            float average = 0;
            for (int i = 0; i < points.size(); i++)
            {
                min = qMin(min, points[i].pos.y());
                max = qMax(max, points[i].pos.y());
                average += points[i].pos.y();
            }
            valueMin = min;
            valueMax = max;
            valueAverage = average / points.size();
        }
    }

public:
    int insertPoint(const CurvePoint& point)
    {
        int index = points.size();
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i] == point)
            {
                points[i] = point;
                index = i;
                break;
            }
            else if (points[i] > point)
            {
                points.insert(i, point);
                index = i;
                break;
            }
        }
        if(index == points.size())
        {
            points.append(point);
        }

        if(points.size() > 1 && index != 0)
        {
            QVector2D pos2 = (points[index].pos + points[index - 1].pos) / 2;
            points[index].pos2 = pos2;
        }
        updateValue();
        return  index;
    }

    void findTouchPoint(const QVector2D& pos)
    {
        int index = 0;
        float min = FLT_MAX;
        for (int i = 0; i < points.size(); i++)
        {
            points[i].touch = false;
            QVector2D d = pos - points[i].pos;
            if(min > d.length())
            {
                min = d.length();
                index = i;
            }
        }
        if(index < points.size())
        {
            points[index].touch = true;
        }
    }

    void leftTouchPoint(TouchType type)
    {
        switch (type) {
        case Touch_Move:
            for (int i = 1; i < points.size(); i++)
            {
                if(points[i].touch)
                {
                    points[i].touch = false;
                    points[i-1].touch = true;
                }
            }
            break;
        case Touch_Add:
            for (int i = 1; i < points.size(); i++)
            {
                if(points[i].touch)
                {
                    points[i-1].touch = true;
                }
            }
            break;
        case Touch_Take:
            bool flag = true;
            for (int i = 0; i < points.size(); i++)
            {
                if(points[i].touch)
                {
                    points[i].touch = flag;
                    flag = false;
                }
            }
            break;
        }
    }

    void rightTouchPoint(TouchType type)
    {
        switch (type) {
        case Touch_Move:
            for (int i = points.size() - 1; i > 0; i--)
            {
                if(points[i-1].touch)
                {
                    points[i-1].touch = false;
                    points[i].touch = true;
                }
            }
            break;
        case Touch_Add:
            for (int i = points.size() - 1; i > 0; i--)
            {
                if(points[i-1].touch)
                {
                    points[i].touch = true;
                }
            }
            break;
        case Touch_Take:
            bool flag = true;
            for (int i = points.size() - 1; i >= 0; i--)
            {
                if(points[i].touch)
                {
                    points[i].touch = flag;
                    flag = false;
                }
            }
            break;
        }
    }

    int deleteTouchPoint()
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                points.removeAt(i--);
                count++;
            }
        }
        updateValue();
        return count;
    }

    int ceilTouchPoint(MoveType type)
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                switch (type) {
                case X_Axis:
                    points[i].pos.setX(ceilf(points[i].pos.x()));
                    break;
                case Y_Axis:
                    points[i].pos.setY(ceilf(points[i].pos.y()));
                    break;
                default:
                    points[i].pos.setX(ceilf(points[i].pos.x()));
                    points[i].pos.setY(ceilf(points[i].pos.y()));
                    break;
                }
                count++;
            }
        }
        updateValue();
        return count;
    }

    int floorTouchPoint(MoveType type)
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                switch (type) {
                case X_Axis:
                    points[i].pos.setX(floorf(points[i].pos.x()));
                    break;
                case Y_Axis:
                    points[i].pos.setY(floorf(points[i].pos.y()));
                    break;
                default:
                    points[i].pos.setX(floorf(points[i].pos.x()));
                    points[i].pos.setY(floorf(points[i].pos.y()));
                    break;
                }
                count++;
            }
        }
        updateValue();
        return count;
    }

    int moveTouchPoint(const QVector2D& offset, MoveType type)
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                switch (type) {
                case X_Axis:
                    points[i].pos.setX(points[i].pos.x() + offset.x());
                    break;
                case Y_Axis:
                    points[i].pos.setY(points[i].pos.y() + offset.y());
                    break;
                default:
                    points[i].pos += offset;
                    break;
                }
                count++;
            }
            if(points[i].touch2)
            {
                points[i].pos2 += offset;
                count++;
            }
        }
        updateValue();
        return count;
    }

    int moveDragPoint(const QVector2D& offset, MoveType type)
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].drag)
            {
                switch (type) {
                case X_Axis:
                    points[i].pos.setX(points[i].pos.x() + offset.x());
                    break;
                case Y_Axis:
                    points[i].pos.setY(points[i].pos.y() + offset.y());
                    break;
                default:
                    points[i].pos += offset;
                    break;
                }
                count++;
            }
            if(points[i].drag2)
            {
                points[i].pos2 += offset;
                count++;
            }
        }
        updateValue();
        return count;
    }

public:
    int touchPoint(const QRectF& rect)
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(rect.contains(points[i].pos.toPointF()))
            {
                points[i].touch = true;
                count++;
            }
            if(rect.contains(points[i].pos2.toPointF()))
            {
                points[i].touch2 = true;
                count++;
            }
        }
        return count;
    }

    int touchPoint(const QVector2D& pos, float scale)
    {
        double offset = static_cast<double>(DotSize / scale);
        QRectF rect;
        rect.setTopLeft(pos.toPointF() - QPointF(offset, offset));
        rect.setBottomRight(pos.toPointF() + QPointF(offset, offset));
        return touchPoint(rect);
    }

    int touchPoint()
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                count++;
            }
            if(points[i].touch2)
            {
                count++;
            }
        }
        return count;
    }

    int dragPoint()
    {
        int count = 0;
        for (int i = 0; i < points.size(); i++)
        {
            if(points[i].touch)
            {
                points[i].drag = true;
                count++;
            }
            if(points[i].touch2)
            {
                points[i].drag2 = true;
                count++;
            }
        }
        return count;
    }


public:
    CurvePoint& firstPoint()
    {
        return points.first();
    }

    CurvePoint& lastPoint()
    {
        return points.last();
    }

    CurvePoint& currentPoint(int i)
    {
        return points[i];
    }

    CurvePoint& evaluatePoint(int i)
    {
        return points[i-1];
    }

    QVector2D evaluate(int i, float t)
    {
        return evaluate(t, currentPoint(i), evaluatePoint(i));
    }

    QVector2D evaluate(float t, const CurvePoint& point, const CurvePoint& pointd)
    {
        if (point.type == CurvePoint::Line)
        {
            const QVector2D& A = point.pos;
            const QVector2D& B = pointd.pos;
            return	(B - A) * t + A;
        }
        else if (point.type == CurvePoint::Curve)
        {
            const QVector2D& A = point.pos;
            const QVector2D& P1 = point.pos2;
            const QVector2D& B = pointd.pos;
            const QVector2D& P2 = point.pos2;
            return	1.0f * powf(t, 3.0f) * (B + 3.0f * (P1 - P2) - A) +
                3.0f * powf(t, 2.0f) * (A - 2.0f * P1 + P2) +
                3.0f * t * (P1 - A) +
                A;
        }
        const QVector2D& A = point.pos;
        return QVector2D(pointd.pos.x(), A.y());
    }

private:
    float valueMin;
    float valueMax;
    float valueAverage;
    QVector<QVariant> records;
    QVector<CurvePoint> points;
};

class QCurveEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QCurveEditWidget(QWidget *parent = nullptr);
    ~QCurveEditWidget() override;

public slots:
    void onTimer();
    void showContextMenu(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void resetView();
    void hideTips();
    void moveType();

    void findPoint();
    void releasePoint();

    void addPoint();
    void addPoint2();
    void deletePoint();

    void upPoint();
    void downPoint();
    void leftPoint();
    void rightPoint();

    void selectdTotalPoint();
    void upContorlPoint();
    void downContorlPoint();
    void leftContorlPoint();
    void rightContorlPoint();

    void leftShiftPoint();
    void rightShiftPoint();

private:
    QPoint toCanvasCoordinates(const QVector2D& analyticPos);
    QVector2D toAnalyticCoordinates(const QPoint& canvasPos);

private:
    QTimer m_timer;
    QPoint m_position;

    bool m_hide;
    bool m_select;
    QRect m_selectGeometry;

    float m_scale;
    QVector2D m_centerOffset;

    QPoint m_dragPosition;
    QVector2D m_dragOffset;

    CurveLines m_curveLines;
    CurveLines::MoveType m_curveMove;
};

#endif // QCURVEEDITWIDGET_H
