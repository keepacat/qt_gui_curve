#ifndef CURVEWIDGET_H
#define CURVEWIDGET_H

#include <cmath>
#include <QWidget>
#include <QVector2D>

enum PointType{
    PointDefault = 0x00,
    PointLine = 0x01,
    PointCurve = 0x02,
    PointFixed = 0xff,
};

struct CurvePoint
{
    PointType type;
    QVector2D pos;
    QVector2D aid;
    bool dot;
    bool doa;

    CurvePoint(PointType t = PointDefault, float x = 0, float y = 0) : type(t), pos(x, y), aid(x, y), dot(false), doa(false) {}

    CurvePoint& operator=(const CurvePoint& p)
    {
        type = p.type;
        pos = p.pos;
        aid = p.aid;
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
        return pos.x() == p.pos.x();
    }
};

struct CurveLines
{
    float value;
    float length;
    QVector<int> drags;
    QVector<CurvePoint> points;

    CurveLines() : value(0), length(0) {}

    void insertPoint(const CurvePoint& point)
    {
        for (int i = 0; i < points.size(); i++)
        {
            if (points[i] > point)
            {
                points.insert(i, point);
                return;
            }
        }
        points.append(point);
    }

    void deletePoint(const CurvePoint& point)
    {
        points.removeOne(point);
    }

    bool hasPoint(const CurvePoint& point)
    {
        return points.indexOf(point) != -1;
    }

    float getValue(float x)
    {
        for (int i = 0; i < points.size(); i++)
        {
            if (points[i].pos.x() == x)
            {
                return points[i].pos.y();
            }
            if (i < points.size() - 1)
            {
                if (points[i].pos.x() < x && points[i+1].pos.x() > x)
                {
                    int ox = points[i + 1].pos.x() - points[i].pos.x();
                    float t = (x - points[i].pos.x()) / ox;
                    return evaluate(i, t).y();
                }
            }
            else {
                return evaluate(i, 0).y();
            }
        }
        return 0;
    }

    QVector2D evaluate(int i, float t)
    {
        if (points.size() == i - 1)
        {
            return QVector2D();
        }
        const CurvePoint& point = points.at(i);
        if (point.type == 0)
        {
            const CurvePoint& pointd = points.at(++i);
            const QVector2D& A = point.pos;
            return QVector2D(pointd.pos.x(), A.y());
        }
        else if (point.type == 1)
        {
            const CurvePoint& pointd = points.at(++i);
            const QVector2D& A = point.pos;
            const QVector2D& B = pointd.pos;
            return	(B - A) * t + A;
        }
        else if (point.type == 2) {
            const CurvePoint& pointd = points.at(++i);
            const QVector2D& A = point.pos;
            const QVector2D& P1 = point.aid;
            const QVector2D& B = pointd.pos;
            const QVector2D& P2 = pointd.aid;
            return	1.0f * pow(t, 3.0f) * (B + 3.0f * (P1 - P2) - A) +
                3.0f * pow(t, 2.0f) * (A - 2.0f * P1 + P2) +
                3.0f * t * (P1 - A) +
                A;
        }
        else {
            const CurvePoint& pointd = points.at(++i);
            const QVector2D& A = point.pos;
            return QVector2D(pointd.pos.x(), A.y());
        }
        return QVector2D(0, 0);
    }

    CurveLines& operator=(const CurveLines& p)
    {
        length = p.length;
        points = p.points;
        return *this;
    }
};

class QCurveWidget : public QWidget
{
    Q_OBJECT
public:
    CurveLines m_curveData;

private:
    QTimer* m_timer;
    QPoint m_position;

    float m_scale{ 100.0f };
    QVector2D m_centerOffset;

    QPoint m_dragPosition;
    QVector2D m_dragOffset;

    bool m_select { false };
    QRect m_selectGeometry;

public:
    explicit QCurveWidget(QWidget* parent = nullptr);
    ~QCurveWidget();

private slots:
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
    void focusView();
    void addPoint();
    void deletePoint();
    void changePoint();

private:
    bool modifyPoint(CurvePoint &point);

private:
    QPoint toCanvasCoordinates(const QVector2D& analyticPos);
    QVector2D toAnalyticCoordinates(const QPoint& canvasPos);
};

#endif // CURVEWIDGET_H
