#include "curvelines.h"
#include <QDebug>

CurveLines::CurveLines() : m_min(0), m_max(0), m_average(0)
{

}

CurveLines::~CurveLines()
{

}

void CurveLines::onCurve(const QVector<CurvePoint> &points)
{
    m_points = points;
}

int CurveLines::pointsSize()
{
    return m_points.size();
}

int CurveLines::pointsTouchSize()
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            count++;
        }
        if(m_points[i].touch2)
        {
            count++;
        }
    }
    return count;
}

int CurveLines::pointsDragSize()
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            m_points[i].drag = true;
            count++;
        }
        if(m_points[i].touch2)
        {
            m_points[i].drag2 = true;
            count++;
        }
    }
    return count;
}

void CurveLines::insertPoint(const CurvePoint &point)
{
    int index = m_points.size();
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i] == point)
        {
            m_points[i] = point;
            index = i;
            break;
        }
        else if (m_points[i] > point)
        {
            m_points.insert(i, point);
            index = i;
            break;
        }
    }
    if(index == m_points.size())
    {
        m_points.append(point);
    }

    if(m_points.size() > 1 && index != 0)
    {
        QVector2D pos2 = (m_points[index].pos + m_points[index - 1].pos) / 2;
        m_points[index].pos2 = pos2;
    }
    updatePoints();
}

void CurveLines::selectPoints()
{
    for (int i = 0; i < m_points.size(); i++)
    {
        m_points[i].drag = false;
        m_points[i].touch = true;
        m_points[i].drag2 = false;
        m_points[i].touch2 = true;
    }
}

void CurveLines::releasePoints()
{
    for (int i = 0; i < m_points.size(); i++)
    {
        m_points[i].drag = false;
        m_points[i].touch = false;
        m_points[i].drag2 = false;
        m_points[i].touch2 = false;
    }
}

void CurveLines::updatePoints()
{
    if(m_points.size())
    {
        float min = FLT_MAX;
        float max = -FLT_MAX;
        float average = 0;
        for (int i = 0; i < m_points.size(); i++)
        {
            min = qMin(min, m_points[i].pos.y());
            max = qMax(max, m_points[i].pos.y());
            average += m_points[i].pos.y();
        }
        m_min = min;
        m_max = max;
        m_average = average / m_points.size();
    }
    emit updateCurve(m_points);
}

float CurveLines::getValue(float x)
{
    for (int i = 0; i < m_points.size(); i++)
    {
        if ((m_points[i].pos.x() - x) <= m_points[i].MinimumFloatMistake)
        {
            return m_points[i].pos.y();
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

float CurveLines::getMinValue()
{
    return m_min;
}

float CurveLines::getMaxValue()
{
    return m_max;
}

float CurveLines::getAverageValue()
{
    return m_average;
}

int CurveLines::touchPoints(const QRectF &rect)
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(rect.contains(m_points[i].pos.toPointF()))
        {
            m_points[i].touch = true;
            count++;
        }
        if(rect.contains(m_points[i].pos2.toPointF()))
        {
            m_points[i].touch2 = true;
            count++;
        }
    }
    return count;
}

int CurveLines::touchPoints(const QVector2D &pos, float scale)
{
    double offset = static_cast<double>(3.0f / scale);
    QRectF rect;
    rect.setTopLeft(pos.toPointF() - QPointF(offset, offset));
    rect.setBottomRight(pos.toPointF() + QPointF(offset, offset));
    return touchPoints(rect);
}

void CurveLines::findTouchPoint(const QVector2D &pos)
{
    int index = 0;
    float min = FLT_MAX;
    for (int i = 0; i < m_points.size(); i++)
    {
        m_points[i].touch = false;
        QVector2D d = pos - m_points[i].pos;
        if(min > d.length())
        {
            min = d.length();
            index = i;
        }
    }
    if(index < m_points.size())
    {
        m_points[index].touch = true;
    }
}

void CurveLines::leftTouchPoint(CurveLines::TouchType type)
{
    switch (type) {
    case Touch_Move:
        for (int i = 1; i < m_points.size(); i++)
        {
            if(m_points[i].touch)
            {
                m_points[i].touch = false;
                m_points[i-1].touch = true;
            }
        }
        break;
    case Touch_Add:
        for (int i = 1; i < m_points.size(); i++)
        {
            if(m_points[i].touch)
            {
                m_points[i-1].touch = true;
            }
        }
        break;
    case Touch_Take:
        bool flag = true;
        for (int i = 0; i < m_points.size(); i++)
        {
            if(m_points[i].touch)
            {
                m_points[i].touch = flag;
                flag = false;
            }
        }
        break;
    }
}

void CurveLines::rightTouchPoint(CurveLines::TouchType type)
{
    switch (type) {
    case Touch_Move:
        for (int i = m_points.size() - 1; i > 0; i--)
        {
            if(m_points[i-1].touch)
            {
                m_points[i-1].touch = false;
                m_points[i].touch = true;
            }
        }
        break;
    case Touch_Add:
        for (int i = m_points.size() - 1; i > 0; i--)
        {
            if(m_points[i-1].touch)
            {
                m_points[i].touch = true;
            }
        }
        break;
    case Touch_Take:
        bool flag = true;
        for (int i = m_points.size() - 1; i >= 0; i--)
        {
            if(m_points[i].touch)
            {
                m_points[i].touch = flag;
                flag = false;
            }
        }
        break;
    }
}

int CurveLines::deleteTouchPoint()
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            m_points.removeAt(i--);
            count++;
        }
    }
    updatePoints();
    return count;
}

int CurveLines::ceilTouchPoint(CurveLines::MoveType type)
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            switch (type) {
            case X_Axis:
                m_points[i].pos.setX(ceilf(m_points[i].pos.x()));
                break;
            case Y_Axis:
                m_points[i].pos.setY(ceilf(m_points[i].pos.y()));
                break;
            default:
                m_points[i].pos.setX(ceilf(m_points[i].pos.x()));
                m_points[i].pos.setY(ceilf(m_points[i].pos.y()));
                break;
            }
            count++;
        }
    }
    updatePoints();
    return count;
}

int CurveLines::floorTouchPoint(CurveLines::MoveType type)
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            switch (type) {
            case X_Axis:
                m_points[i].pos.setX(floorf(m_points[i].pos.x()));
                break;
            case Y_Axis:
                m_points[i].pos.setY(floorf(m_points[i].pos.y()));
                break;
            default:
                m_points[i].pos.setX(floorf(m_points[i].pos.x()));
                m_points[i].pos.setY(floorf(m_points[i].pos.y()));
                break;
            }
            count++;
        }
    }
    updatePoints();
    return count;
}

int CurveLines::moveTouchPoint(const QVector2D &offset, CurveLines::MoveType type)
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].touch)
        {
            switch (type) {
            case X_Axis:
                m_points[i].pos.setX(m_points[i].pos.x() + offset.x());
                break;
            case Y_Axis:
                m_points[i].pos.setY(m_points[i].pos.y() + offset.y());
                break;
            default:
                m_points[i].pos += offset;
                break;
            }
            count++;
        }
        if(m_points[i].touch2)
        {
            m_points[i].pos2 += offset;
            count++;
        }
    }
    updatePoints();
    return count;
}

int CurveLines::moveDragPoint(const QVector2D &offset, CurveLines::MoveType type)
{
    int count = 0;
    for (int i = 0; i < m_points.size(); i++)
    {
        if(m_points[i].drag)
        {
            switch (type) {
            case X_Axis:
                m_points[i].pos.setX(m_points[i].pos.x() + offset.x());
                break;
            case Y_Axis:
                m_points[i].pos.setY(m_points[i].pos.y() + offset.y());
                break;
            default:
                m_points[i].pos += offset;
                break;
            }
            count++;
        }
        if(m_points[i].drag2)
        {
            m_points[i].pos2 += offset;
            count++;
        }
    }
    updatePoints();
    return count;
}

CurvePoint &CurveLines::firstPoint()
{
    return m_points.first();
}

CurvePoint &CurveLines::lastPoint()
{
    return m_points.last();
}

CurvePoint &CurveLines::currentPoint(int i)
{
    return m_points[i];
}

CurvePoint &CurveLines::evaluatePoint(int i)
{
    return m_points[i-1];
}

QVector2D CurveLines::evaluate(int i, float t)
{
    return evaluate(t, currentPoint(i), evaluatePoint(i));
}

QVector2D CurveLines::evaluate(float t, const CurvePoint &point, const CurvePoint &pointd)
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














