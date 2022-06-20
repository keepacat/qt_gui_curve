#include "qcurveeditwidget.h"
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QApplication>
#include <QWheelEvent>

const int DotSize = 3;
const int GridWidth = 1;
const QColor DotColor(255, 255, 255);
const QColor DotEdgeColor(0, 0, 0);
const QColor DotSelectionColor(255, 255, 255);
const QColor DotEdgeSelectionColor(255, 0, 0);
const QColor LineColor(237,138,63);
const QColor Line2Color(129,52,175);

QCurveEditWidget::QCurveEditWidget(QWidget *parent) :
    QWidget(parent), m_hide(false), m_select(false), m_scale(100.0f), m_curveMove(CurveLines::Y_Axis)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(38, 38, 38));
    setPalette(pal);
    setAutoFillBackground(true);

    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &QCurveEditWidget::showContextMenu);
}

QCurveEditWidget::~QCurveEditWidget()
{

}

CurveLines *QCurveEditWidget::getCurveLines()
{
    return &m_curveLines;
}

void QCurveEditWidget::addCurveLine(const CurvePoint &point)
{
    m_curveLines.insertPoint(point);
}

void QCurveEditWidget::clearCurveLines()
{
    m_curveLines.selectPoints();
    m_curveLines.deleteTouchPoint();
}

void QCurveEditWidget::resetView()
{
    m_centerOffset = QVector2D(size().width() / 2, size().height() / 2);
    repaint();
}

void QCurveEditWidget::remoteView()
{
    repaint();
}

void QCurveEditWidget::hideTips()
{
    m_hide = !m_hide;
    repaint();
}

void QCurveEditWidget::moveType()
{
    switch (m_curveMove) {
    case CurveLines::X_Axis:
        m_curveMove = CurveLines::Y_Axis;
        break;
    case CurveLines::Y_Axis:
        m_curveMove = CurveLines::XY_Axis;
        break;
    case CurveLines::XY_Axis:
        m_curveMove = CurveLines::X_Axis;
        break;
    }
    repaint();
}

void QCurveEditWidget::findPoint()
{
    QPoint pos = this->mapFromGlobal(QCursor().pos());
    m_curveLines.findTouchPoint(toAnalyticCoordinates(pos));
    if(m_curveLines.pointsTouchSize())
    {
        setCursor(Qt::PointingHandCursor);
        repaint();
    }
}

void QCurveEditWidget::releasePoint()
{
    m_curveLines.releasePoints();
    setCursor(Qt::ArrowCursor);
    repaint();
}

void QCurveEditWidget::addPoint()
{
    QPoint pos = this->mapFromGlobal(QCursor().pos());
    CurvePoint point(toAnalyticCoordinates(pos), CurvePoint::Line);
    m_curveLines.insertPoint(point);
    repaint();
}

void QCurveEditWidget::addPoint2()
{
    QPoint pos = this->mapFromGlobal(QCursor().pos());
    CurvePoint point(toAnalyticCoordinates(pos), CurvePoint::Curve);
    m_curveLines.insertPoint(point);
    repaint();
}

void QCurveEditWidget::deletePoint()
{
    m_curveLines.deleteTouchPoint();
    repaint();
}

void QCurveEditWidget::upPoint()
{
    QVector2D offset(0, 1);
    m_curveLines.moveTouchPoint(offset, m_curveMove);
    repaint();
}

void QCurveEditWidget::downPoint()
{
    QVector2D offset(0, -1);
    m_curveLines.moveTouchPoint(offset, m_curveMove);
    repaint();
}

void QCurveEditWidget::leftPoint()
{
    m_curveLines.leftTouchPoint(CurveLines::Touch_Move);
    repaint();
}

void QCurveEditWidget::rightPoint()
{
    m_curveLines.rightTouchPoint(CurveLines::Touch_Move);
    repaint();
}

void QCurveEditWidget::selectdTotalPoint()
{
    m_curveLines.selectPoints();
    if(m_curveLines.pointsTouchSize())
    {
        setCursor(Qt::PointingHandCursor);
        repaint();
    }
}

void QCurveEditWidget::upContorlPoint()
{
    m_curveLines.ceilTouchPoint(m_curveMove);
    repaint();
}

void QCurveEditWidget::downContorlPoint()
{
    m_curveLines.floorTouchPoint(m_curveMove);
    repaint();
}

void QCurveEditWidget::leftContorlPoint()
{
    m_curveLines.leftTouchPoint(CurveLines::Touch_Add);
    repaint();
}

void QCurveEditWidget::rightContorlPoint()
{
    m_curveLines.rightTouchPoint(CurveLines::Touch_Add);
    repaint();
}

void QCurveEditWidget::leftShiftPoint()
{
    m_curveLines.leftTouchPoint(CurveLines::Touch_Take);
    repaint();
}

void QCurveEditWidget::rightShiftPoint()
{
    m_curveLines.rightTouchPoint(CurveLines::Touch_Take);
    repaint();
}

void QCurveEditWidget::onTimer()
{

}

void QCurveEditWidget::showContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    releasePoint();
}

void QCurveEditWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Horizontal lines
    painter.setPen(QPen(QColor(46, 46, 46), GridWidth, Qt::SolidLine, Qt::FlatCap));
    float y = toAnalyticCoordinates(QPoint(0, 0)).y();
    int y_min = static_cast<int>(ceilf(y));
    y = toAnalyticCoordinates(QPoint(0, size().height())).y();
    int y_max = static_cast<int>(floorf(y));
    for (int i = y_max; i <= y_min; i++)
    {
        int pp = toCanvasCoordinates(QVector2D(0, i)).y();
        painter.drawLine(0, pp, size().width(), pp);
    }

    // Vertical lines
    painter.setPen(QPen(QColor(46, 46, 46), GridWidth, Qt::SolidLine, Qt::FlatCap));
    float x = toAnalyticCoordinates(QPoint(0, 0)).x();
    int x_min = static_cast<int>(ceilf(x));
    x = toAnalyticCoordinates(QPoint(size().width(), 0)).x();
    int x_max = static_cast<int>(floorf(x));
    for (int i = x_min; i <= x_max; i++)
    {
        int pp = toCanvasCoordinates(QVector2D(i, 0)).x();
        painter.drawLine(pp, 0, pp, size().height());
    }

    // Geometry selected
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(90, 90, 200, 150));
    painter.drawRect(m_selectGeometry);

    // Axis
    painter.setPen(QPen(QColor(80, 80, 80), GridWidth, Qt::SolidLine, Qt::FlatCap));
    QPoint p = toCanvasCoordinates(QVector2D(0, 0));
    if (0 <= p.x() && p.x() <= size().width())
        painter.drawLine(p.x(), 0, p.x(), size().height());
    if (0 <= p.y() && p.y() <= size().height())
        painter.drawLine(0, p.y(), size().width(), p.y());

    // Tips
    double length = 0;
    if(m_curveLines.pointsSize() > 1)
    {
        length = static_cast<double>(m_curveLines.lastPoint().pos.x()) - static_cast<double>(m_curveLines.firstPoint().pos.x());
    }
    QStringList tips;
    tips << tr("Count:%1").arg(m_curveLines.pointsSize());
    tips << tr("Length:%1").arg(length);
    tips << tr("Max:%1").arg(static_cast<double>(m_curveLines.getMaxValue()));
    tips << tr("Min:%1").arg(static_cast<double>(m_curveLines.getMinValue()));
    tips << tr("Average:%1").arg(static_cast<double>(m_curveLines.getAverageValue()));
    switch (m_curveMove) {
    case CurveLines::X_Axis:
        tips << tr("MoveType:X_Axis");
        break;
    case CurveLines::Y_Axis:
        tips << tr("MoveType:Y_Axis");
        break;
    case CurveLines::XY_Axis:
        tips << tr("MoveType:XY_Axis");
        break;
    }
    tips << tr("\n");
    if(m_hide)
    {
        tips << tr("press H for a keys tips");
    }
    else{
        tips << tr("Key_H:hide keys tips");
        tips << tr("Key_M:change move type");
        tips << tr("Key_A:add line point");
        tips << tr("Key_C:add curve point");
        tips << tr("Key_D:delete selected point");
        tips << tr("Key_Up:point move up");
        tips << tr("Key_Down:point move down");
        tips << tr("Key_Left:focus move left");
        tips << tr("Key_Right:focus move right");
        tips << tr("Key_Space:find near point");
    }
    painter.setPen(QPen(QColor(200, 200, 200), GridWidth, Qt::SolidLine, Qt::FlatCap));
    painter.drawText(10, 10, size().width(), size().height(), Qt::AlignLeft | Qt::AlignTop, tips.join("\n"));


    // Curve lines
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (int i = 1; i < m_curveLines.pointsSize(); i++)
    {
        const CurvePoint& point = m_curveLines.currentPoint(i);
        const CurvePoint& pointd = m_curveLines.evaluatePoint(i);
        if (point.type == CurvePoint::Line)
        {
            painter.setPen(QPen(LineColor, 2, Qt::SolidLine, Qt::FlatCap));
            painter.drawLine(toCanvasCoordinates(point.pos), toCanvasCoordinates(pointd.pos));
        }
        else if (point.type == CurvePoint::Curve)
        {
            painter.setPen(QPen(Line2Color, 2, Qt::SolidLine, Qt::FlatCap));
            constexpr float steps = 40.0f;
            constexpr float step = 1.0f / steps;
            for (float t = step; t <= 1.0f; t += step)
            {
                QVector2D p0 = m_curveLines.evaluate(t - step, point, pointd);
                QVector2D p1 = m_curveLines.evaluate(t, point, pointd);
                painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));
            }
            {
                QVector2D p0 = m_curveLines.evaluate(1.0f - step, point, pointd);
                QVector2D p1 = m_curveLines.evaluate(1.0f, point, pointd);
                painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));
            }

            QColor col = point.touch2 ? DotSelectionColor : DotColor;
            QColor colEdge = point.touch2 ? DotEdgeSelectionColor : DotEdgeColor;

            painter.setPen(QPen(colEdge, 1, Qt::SolidLine, Qt::FlatCap));
            painter.setBrush(QBrush(col));

            QPoint center = toCanvasCoordinates(point.pos2);
            painter.drawEllipse(center.x() - DotSize, center.y() - DotSize, DotSize * 2, DotSize * 2);
        }
        else {
            painter.setPen(QPen(DotColor, 2, Qt::SolidLine, Qt::FlatCap));
            QVector2D p0(point.pos.x(), 0);
            QVector2D p1(pointd.pos.x(), 0);
            painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));

            painter.setPen(QPen(DotColor, 2, Qt::DotLine, Qt::FlatCap));
            painter.drawLine(toCanvasCoordinates(point.pos), toCanvasCoordinates(p0));
            painter.drawLine(toCanvasCoordinates(pointd.pos), toCanvasCoordinates(p1));
        }
    }
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dots
    const int spacWidth = DotSize * 8;
    const int spacHeight = DotSize * 2;
    for (int i = 0; i < m_curveLines.pointsSize(); i++)
    {
        const CurvePoint& point = m_curveLines.currentPoint(i);

        QColor col = point.touch ? DotSelectionColor : DotColor;
        QColor colEdge = point.touch ? DotEdgeSelectionColor : DotEdgeColor;

        painter.setPen(QPen(colEdge, 1, Qt::SolidLine, Qt::FlatCap));
        painter.setBrush(QBrush(col));

        QPoint center = toCanvasCoordinates(point.pos);
        painter.drawRect(center.x() - DotSize, center.y() - DotSize, DotSize * 2, DotSize * 2);

        QColor pcol = point.touch ? DotEdgeSelectionColor : DotColor;
        painter.setPen(QPen(pcol));
        if(point.touch)
        {
            QRect pointX(center.x() - spacWidth, spacHeight, spacWidth * 2, spacHeight * 2);
            painter.drawText(pointX, Qt::AlignCenter, QString::number(static_cast<double>(point.pos.x()), 'f', 1));
            QRect pointY(center.x() - spacWidth, center.y() - spacHeight * 4, spacWidth * 2, spacHeight * 2);
            painter.drawText(pointY, Qt::AlignCenter, QString::number(static_cast<double>(point.pos.y()), 'f', 3));
        }
        else {
            if(i == 0 || i == m_curveLines.pointsSize() - 1)
            {
                QRect pointX(center.x() - spacWidth, spacHeight, spacWidth * 2, spacHeight * 2);
                painter.drawText(pointX, Qt::AlignCenter, QString::number(static_cast<double>(point.pos.x()), 'f', 1));
            }
        }
    }

    QWidget::paintEvent(event);
}

void QCurveEditWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::MiddleButton)
    {
        m_dragPosition = event->pos();
        m_dragOffset = m_centerOffset;
    }
    else if (event->button() == Qt::MouseButton::LeftButton)
    {
        const QVector2D &pos = toAnalyticCoordinates(event->pos());
        if (m_curveLines.touchPoints(pos, m_scale))
        {
            setCursor(Qt::PointingHandCursor);
            repaint();
        }
        if(m_curveLines.pointsDragSize() == 0)
        {
            m_select = true;
            m_selectGeometry.setTopLeft(event->pos());
            m_selectGeometry.setBottomRight(event->pos());
        }
    }
    QWidget::mousePressEvent(event);
}

void QCurveEditWidget::mouseMoveEvent(QMouseEvent *event)
{
    QVector2D oldPos = toAnalyticCoordinates(m_position);
    m_position = event->pos();

    if (event->buttons() == Qt::MouseButton::MiddleButton)
    {
        m_centerOffset = m_dragOffset + QVector2D(event->pos()) - QVector2D(m_dragPosition);
        repaint();
    }
    else if (event->buttons() == Qt::MouseButton::LeftButton)
    {
        if (m_select)
        {
            m_selectGeometry.setBottomRight(event->pos());
            repaint();
        }
        else{
            QVector2D newPos = toAnalyticCoordinates(m_position);
            QVector2D offset = newPos - oldPos;
            if(m_curveLines.moveDragPoint(offset, m_curveMove))
            {
                repaint();
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}

void QCurveEditWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton)
    {
        if (m_select)
        {
            auto topLeft = toAnalyticCoordinates(m_selectGeometry.topLeft());
            auto bottomRight = toAnalyticCoordinates(m_selectGeometry.bottomRight());
            QRectF rect(topLeft.toPointF(), bottomRight.toPointF());
            m_curveLines.touchPoints(rect);

            m_select = false;
            m_selectGeometry.setTopLeft(QPoint(0, 0));
            m_selectGeometry.setBottomRight(QPoint(0, 0));
        }
        else{
            if(QApplication::keyboardModifiers() != Qt::ControlModifier)
            {
                m_curveLines.releasePoints();
            }
        }
        repaint();
    }
    if(m_curveLines.pointsTouchSize())
    {
        setCursor(Qt::PointingHandCursor);
    }
    else{
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}

void QCurveEditWidget::wheelEvent(QWheelEvent *event)
{
    if (QApplication::mouseButtons() != Qt::MouseButton::MiddleButton)
    {
        QPoint pos = event->pos();
        float scale = m_scale;

        int numDegrees = event->delta();
        m_scale *= float((numDegrees > 0) ? 1.05f : 0.95f);

        float ox = (pos.x() * (scale - m_scale) + m_centerOffset.x() * m_scale) / scale;
        float oy = (pos.y() * (scale - m_scale) + m_centerOffset.y() * m_scale) / scale;
        m_centerOffset.setX(ox);
        m_centerOffset.setY(oy);
    }
    event->accept();
    repaint();
    QWidget::wheelEvent(event);
}

void QCurveEditWidget::resizeEvent(QResizeEvent *event)
{
    float proportionX = static_cast<float>(event->size().width()) / event->oldSize().width();
    float proportionY = static_cast<float>(event->size().height()) / event->oldSize().height();

    if (proportionX >= 0.0001f && proportionY >= 0.0001f)
    {
        m_centerOffset = QVector2D(m_centerOffset.x() * proportionX, m_centerOffset.y() * proportionY);
    }
    QWidget::resizeEvent(event);
}

void QCurveEditWidget::showEvent(QShowEvent *event)
{
    resetView();
    QWidget::showEvent(event);
}

void QCurveEditWidget::closeEvent(QCloseEvent *event)
{
    emit closeWidget();
    QWidget::closeEvent(event);
}

void QCurveEditWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::NoModifier)
    {
        switch (event->key())
        {
        case Qt::Key_R:
            resetView();
            break;
        case Qt::Key_O:
            remoteView();
            break;
        case Qt::Key_H:
            hideTips();
            break;
        case Qt::Key_M:
            moveType();
            break;
        case Qt::Key_A:
            addPoint();
            break;
        case Qt::Key_C:
            addPoint2();
            break;
        case Qt::Key_D:
        case Qt::Key_Delete:
            deletePoint();
            break;
        case Qt::Key_Up:
            upPoint();
            break;
        case Qt::Key_Down:
            downPoint();
            break;
        case Qt::Key_Left:
            leftPoint();
            break;
        case Qt::Key_Right:
            rightPoint();
            break;
        case Qt::Key_Space:
            findPoint();
            break;
        case Qt::Key_Escape:
            releasePoint();
            break;
        default:
            break;
        }
    }
    else if(event->modifiers() == Qt::ControlModifier)
    {
        switch (event->key())
        {
        case Qt::Key_A:
            selectdTotalPoint();
            break;
        case Qt::Key_Up:
            upContorlPoint();
            break;
        case Qt::Key_Down:
            downContorlPoint();
            break;
        case Qt::Key_Left:
            leftContorlPoint();
            break;
        case Qt::Key_Right:
            rightContorlPoint();
            break;
        default:
            break;
        }
    }
    else if(event->modifiers() == Qt::ShiftModifier)
    {
        switch (event->key())
        {
        case Qt::Key_Left:
            leftShiftPoint();
            break;
        case Qt::Key_Right:
            rightShiftPoint();
            break;
        default:
            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void QCurveEditWidget::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
}

QPoint QCurveEditWidget::toCanvasCoordinates(const QVector2D &analyticPos)
{
    float x = analyticPos.x() * m_scale + m_centerOffset.x();
    float y = -analyticPos.y() * m_scale + m_centerOffset.y();
    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

QVector2D QCurveEditWidget::toAnalyticCoordinates(const QPoint &canvasPos)
{
    float x = (canvasPos.x() - m_centerOffset.x()) / m_scale;
    float y = (-canvasPos.y() + m_centerOffset.y()) / m_scale;
    return QVector2D(x, y);
}
