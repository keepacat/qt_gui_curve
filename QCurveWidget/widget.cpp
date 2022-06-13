#include "widget.h"
#include <QTimer>
#include <QRect>
#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QPainter>
#include <QDialog>
#include <QTableWidget>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QHeaderView>

const int MinimumSelectDotsDistance = 9;
const QColor DotEdgeColor(0, 0, 0);
const QColor DotColor(255, 255, 255);
const QColor DotEdgeSelectionColor(255, 0, 0);
const QColor DotSelectionColor(255, 255, 255);
const QColor GridThinColor(46, 46, 46);
const QColor FatGridColor(80, 80, 80);
const int GridWidth = 1;
const int DotSize = 3;
const int RepaintFps = 60;

QCurveWidget::QCurveWidget(QWidget* parent) : QWidget(parent)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(38, 38, 38));

    setPalette(pal);
    setAutoFillBackground(true);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_timer = new QTimer(this);
    m_timer->start(1000 / RepaintFps);
    connect(m_timer, &QTimer::timeout, this, &QCurveWidget::onTimer);
    connect(this, &QWidget::customContextMenuRequested, this, &QCurveWidget::showContextMenu);

    CurvePoint point;
    point.type = PointFixed;
    point.pos = toAnalyticCoordinates(QPoint(0, 0));
    point.aid = toAnalyticCoordinates(QPoint(0, 0));
    m_curveLines.insertPoint(point);
}

QCurveWidget::~QCurveWidget()
{

}

void QCurveWidget::onTimer()
{
    auto check_dot_intersection = [&](const QVector2D& P) -> bool
    {
        auto dist = (m_position - toCanvasCoordinates(P)).manhattanLength();
        return (dist < MinimumSelectDotsDistance);
    };

    bool isRepaint = false;
    for (int i = 0; i < m_curveLines.points.size(); i++)
    {
        if (m_curveLines.drags.indexOf(i) != -1)
        {
            continue;
        }
        CurvePoint& point = m_curveLines.currentPoint(i);
        if (point.type == PointFixed)
        {
            continue;
        }
        else if (point.type == PointCurve)
        {
            auto doa = check_dot_intersection(point.aid);
            if (doa != point.doa)
            {
                point.doa = doa;
                isRepaint = true;
            }
            if(i > 0)
            {
                CurvePoint& point2 = m_curveLines.evaluatePoint(i);
                auto doa2 = check_dot_intersection(point2.aid);
                if (doa2 != point2.doa)
                {
                    point2.doa = doa2;
                    isRepaint = true;
                }
            }
        }
        auto dot = check_dot_intersection(point.pos);
        if (dot != point.dot)
        {
            point.dot = dot;
            isRepaint = true;
        }
    }
    if (isRepaint)
    {
        repaint();
    }
}

void QCurveWidget::showContextMenu(const QPoint& pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    QAction actionR("reset view", this);
    connect(&actionR , &QAction::triggered, this, &QCurveWidget::resetView);

    QAction actionF("focus view", this);
    connect(&actionF, &QAction::triggered, this, &QCurveWidget::focusView);

    QAction actionA("add point", this);
    connect(&actionA, &QAction::triggered, this, &QCurveWidget::addPoint);

    QAction actionD("delete point", this);
    connect(&actionD, &QAction::triggered, this, &QCurveWidget::deletePoint);

    QAction actionC("change point", this);
    connect(&actionC, &QAction::triggered, this, &QCurveWidget::changePoint);

    contextMenu.addAction(&actionR);
    contextMenu.addAction(&actionF);
    contextMenu.addAction(&actionA);
    contextMenu.addAction(&actionD);
    contextMenu.addAction(&actionC);
    contextMenu.exec(mapToGlobal(pos));
}

void QCurveWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(QPen(GridThinColor, GridWidth, Qt::SolidLine, Qt::FlatCap));

    // Horizontal lines
    float y = toAnalyticCoordinates(QPoint(0, 0)).y();
    int y_min = static_cast<int>(ceil(y));
    y = toAnalyticCoordinates(QPoint(0, size().height())).y();
    int y_max = static_cast<int>(floor(y));
    for (int i = y_max; i <= y_min; i++)
    {
        int pp = toCanvasCoordinates(QVector2D(0, i)).y();
        painter.drawLine(0, pp, size().width(), pp);
    }

    // Vertical lines
    float x = toAnalyticCoordinates(QPoint(0, 0)).x();
    int x_min = static_cast<int>(ceil(x));
    x = toAnalyticCoordinates(QPoint(size().width(), 0)).x();
    int x_max = static_cast<int>(floor(x));
    for (int i = x_min; i <= x_max; i++)
    {
        int pp = toCanvasCoordinates(QVector2D(i, 0)).x();
        painter.drawLine(pp, 0, pp, size().height());
    }

    // Axis
    painter.setPen(QPen(FatGridColor, GridWidth, Qt::SolidLine, Qt::FlatCap));
    QPoint p = toCanvasCoordinates(QVector2D(0, 0));
    if (0 <= p.x() && p.x() <= size().width())
        painter.drawLine(p.x(), 0, p.x(), size().height());
    if (0 <= p.y() && p.y() <= size().height())
        painter.drawLine(0, p.y(), size().width(), p.y());

    // select geometry
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 100));
    painter.drawRect(m_selectGeometry);

    // Points
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (int i = 1; i < m_curveLines.points.size(); i++)
    {
        const CurvePoint& point = m_curveLines.currentPoint(i);
        if (point.type == PointLine)
        {
            painter.setPen(QPen(QColor("#ED8A3F"), 2, Qt::SolidLine, Qt::FlatCap));
            QVector2D p0 = point.pos;
            QVector2D p1 = m_curveLines.evaluate(i, 1);
            painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));
        }
        else if (point.type == PointCurve)
        {
            painter.setPen(QPen(QColor("#8134af"), 2, Qt::SolidLine, Qt::FlatCap));
            constexpr float steps = 40.0f;
            constexpr float step = 1.0f / steps;
            for (float t = step; t <= 1.0f; t += step)
            {
                QVector2D p0 = m_curveLines.evaluate(i, t - step);
                QVector2D p1 = m_curveLines.evaluate(i, t);
                painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));
            }
            {
                QVector2D p0 = m_curveLines.evaluate(i, 1.0f - step);
                QVector2D p1 = m_curveLines.evaluate(i, 1.0f);
                painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));
            }
            painter.setPen(QPen(Qt::green, 1, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(toCanvasCoordinates(point.pos), toCanvasCoordinates(point.aid));
            const CurvePoint& point2 = m_curveLines.evaluatePoint(i);
            painter.drawLine(toCanvasCoordinates(point2.pos), toCanvasCoordinates(point2.aid));
        }
        else {
            const CurvePoint& point2 = m_curveLines.evaluatePoint(i);
            painter.setPen(QPen(QColor("#fafbfd"), 2, Qt::SolidLine, Qt::FlatCap));
            QVector2D p0(point.pos.x(), m_curveLines.value);
            QVector2D p1(point2.pos.x(), m_curveLines.value);
            painter.drawLine(toCanvasCoordinates(p0), toCanvasCoordinates(p1));

            painter.setPen(QPen(QColor("#fafbfd"), 2, Qt::DotLine, Qt::FlatCap));
            painter.drawLine(toCanvasCoordinates(point.pos), toCanvasCoordinates(p0));
            painter.drawLine(toCanvasCoordinates(point2.pos), toCanvasCoordinates(p1));
        }
    }
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dots
    const int spacWidth = DotSize * 8;
    const int spacHeight = DotSize * 2;
    const int spec = spacWidth > m_scale ? (spacWidth*2 / m_scale) : 1;
    //qDebug() << spec << spacWidth << m_scale;
    for (int i = 0; i < m_curveLines.points.size(); i++)
    {
        const CurvePoint& point = m_curveLines.currentPoint(i);

        QColor col = point.dot ? DotSelectionColor : DotColor;
        QColor colEdge = point.dot ? DotEdgeSelectionColor : DotEdgeColor;

        painter.setPen(QPen(colEdge, 1, Qt::SolidLine, Qt::FlatCap));
        painter.setBrush(QBrush(col));

        QPoint center = toCanvasCoordinates(point.pos);
        painter.drawRect(center.x() - DotSize, center.y() - DotSize, DotSize * 2, DotSize * 2);

        QColor pcol = point.dot ? DotEdgeSelectionColor : DotColor;
        painter.setPen(QPen(pcol));
        if(point.dot || m_scale > 30)
        {
            painter.drawText(center.x() - spacWidth, spacHeight, spacWidth * 2, spacHeight * 2, Qt::AlignCenter, QString::number(point.pos.x()));
            painter.drawText(center.x() - spacWidth, center.y() - spacHeight * 4, spacWidth * 2, spacHeight * 2, Qt::AlignCenter, QString::number(point.pos.y()));
        }
        else {
            if (i % spec == 0)
            {
                painter.drawText(center.x() - spacWidth, spacHeight, spacWidth * 2, spacHeight * 2, Qt::AlignCenter, QString::number(point.pos.x()));
            }
        }

        if (point.type == PointCurve)
        {
            col = point.doa ? DotSelectionColor : DotColor;
            colEdge = point.doa ? DotEdgeSelectionColor : DotEdgeColor;

            painter.setPen(QPen(colEdge, 1, Qt::SolidLine, Qt::FlatCap));
            painter.setBrush(QBrush(col));

            center = toCanvasCoordinates(point.aid);
            painter.drawRect(center.x() - DotSize, center.y() - DotSize, DotSize * 2, DotSize * 2);

            if (i > 0)
            {
                const CurvePoint& point2 = m_curveLines.evaluatePoint(i);

                col = point2.doa ? DotSelectionColor : DotColor;
                colEdge = point2.doa ? DotEdgeSelectionColor : DotEdgeColor;

                painter.setPen(QPen(colEdge, 1, Qt::SolidLine, Qt::FlatCap));
                painter.setBrush(QBrush(col));

                center = toCanvasCoordinates(point2.aid);
                painter.drawRect(center.x() - DotSize, center.y() - DotSize, DotSize * 2, DotSize * 2);
            }
        }
    }
}

void QCurveWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::MiddleButton)
    {
        m_dragPosition = event->pos();
        m_dragOffset = m_centerOffset;
    }
    else if (event->button() == Qt::MouseButton::LeftButton)
    {
        for (int i = 0; i < m_curveLines.points.size(); i++)
        {
            CurvePoint& point = m_curveLines.points[i];
            if (point.dot || point.doa)
            {
                if (m_curveLines.drags.indexOf(i) == -1)
                {
                    m_curveLines.drags.append(i);
                }
                return;
            }
        }
        m_select = true;
        m_selectGeometry.setTopLeft(event->pos());
        m_selectGeometry.setBottomRight(event->pos());
    }
}

void QCurveWidget::mouseMoveEvent(QMouseEvent* event)
{
    auto oldPos = toAnalyticCoordinates(m_position);
    m_position = event->pos();
    if (QApplication::mouseButtons() == Qt::MouseButton::MiddleButton)
    {
        m_centerOffset = m_dragOffset + QVector2D(event->pos()) - QVector2D(m_dragPosition);
        repaint();
    }
    else if (QApplication::mouseButtons() == Qt::MouseButton::LeftButton)
    {
        if (!m_select)
        {
            auto newPos = toAnalyticCoordinates(m_position);
            auto offset = newPos - oldPos;
            for (int i = 0; i < m_curveLines.drags.size(); i++)
            {
                CurvePoint& point = m_curveLines.points[m_curveLines.drags[i]];
                if (point.doa)
                {
                    point.aid.setY(point.aid.y() + offset.y());
                }
                else if (point.dot)
                {
                    point.pos.setY(point.pos.y() + offset.y());
                }
            }
            repaint();
        }
        else {
            m_selectGeometry.setBottomRight(event->pos());
            repaint();
        }
    }
}

void QCurveWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_select)
    {
        for (int i = 0; i < m_curveLines.points.size(); i++)
        {
            CurvePoint& point = m_curveLines.points[i];
            if(point.type == PointFixed)
            {
                continue;
            }
            QPoint pos = toCanvasCoordinates(point.pos);
            if (m_selectGeometry.contains(pos, true))
            {
                point.dot = true;
                if (m_curveLines.drags.indexOf(i) == -1)
                {
                    m_curveLines.drags.append(i);
                }
            }
            QPoint pos2 = toCanvasCoordinates(point.aid);
            if (m_selectGeometry.contains(pos2, true))
            {
                point.doa = true;
                if (m_curveLines.drags.indexOf(i) == -1)
                {
                    m_curveLines.drags.append(i);
                }
            }
        }
        m_select = false;
        m_selectGeometry.setTopLeft(QPoint(0, 0));
        m_selectGeometry.setBottomRight(QPoint(0, 0));
        repaint();
    }
    else{
        if (event->button() == Qt::MouseButton::LeftButton)
        {
            m_curveLines.drags.clear();
        }
    }
}

void QCurveWidget::wheelEvent(QWheelEvent* event)
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
}

void QCurveWidget::resizeEvent(QResizeEvent* event)
{
    float proportionX = static_cast<float>(event->size().width()) / event->oldSize().width();
    float proportionY = static_cast<float>(event->size().height()) / event->oldSize().height();

    if (proportionX >= 0.0001f && proportionY >= 0.0001f)
    {
        m_centerOffset = QVector2D(m_centerOffset.x() * proportionX, m_centerOffset.y() * proportionY);
    }
    QWidget::resizeEvent(event);
}

void QCurveWidget::showEvent(QShowEvent* event)
{
    resetView();
    QWidget::showEvent(event);
}

void QCurveWidget::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
}

void QCurveWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier)
    {
        switch (event->key())
        {
        case Qt::Key_R:
            resetView();
            break;
        case Qt::Key_F:
            focusView();
            break;
        case Qt::Key_A:
            addPoint();
            break;
        case Qt::Key_D:
            deletePoint();
            break;
        case Qt::Key_C:
            changePoint();
            break;
        default:
            break;
        }
    }
    QWidget::keyPressEvent(event);
}

void QCurveWidget::keyReleaseEvent(QKeyEvent* event)
{
    QWidget::keyReleaseEvent(event);
}

void QCurveWidget::resetView()
{
    if (m_curveLines.length)
    {
        m_scale = this->width() / (m_curveLines.length * 1.0f);
        m_centerOffset.setX(50);
        m_centerOffset.setY(this->height() - 50);
    }
    else {
        m_scale = 100.0f;
        m_centerOffset.setX(this->width() / 2);
        m_centerOffset.setY(this->height() / 2);
    }
    repaint();
}

void QCurveWidget::focusView()
{
    QPoint pos = this->mapFromGlobal(QCursor().pos());
    if (m_scale == 10.0f)
    {
        pos.setX(pos.x() - this->width() / 2);
        pos.setY(pos.y() - this->height() / 2);
        m_centerOffset.setX(m_centerOffset.x() - pos.x());
        m_centerOffset.setY(m_centerOffset.y() - pos.y());
    }
    else {
        float scale = m_scale;
        m_scale = 10.0f;
        float ox = (pos.x() * (scale - m_scale) + m_centerOffset.x() * m_scale) / scale;
        float oy = (pos.y() * (scale - m_scale) + m_centerOffset.y() * m_scale) / scale;
        m_centerOffset.setX(ox);
        m_centerOffset.setY(oy);
    }
    repaint();
}

void QCurveWidget::addPoint()
{
    m_curveLines.drags.clear();
    QPoint pos = this->mapFromGlobal(QCursor().pos());
    CurvePoint point;
    point.type = PointLine;
    point.pos = toAnalyticCoordinates(pos);
    point.aid = toAnalyticCoordinates(pos);
    if(modifyPoint(point))
    {
        m_curveLines.insertPoint(point);
    }
    repaint();
}

void QCurveWidget::deletePoint()
{
    QVector<CurvePoint> delPoint;
    for (int i = 0; i < m_curveLines.drags.size(); i++)
    {
        int index = m_curveLines.drags[i];
        delPoint.append(m_curveLines.points[index]);
    }
    for (const CurvePoint& point : delPoint)
    {
        m_curveLines.deletePoint(point);
    }
    m_curveLines.drags.clear();
    repaint();
}

void QCurveWidget::changePoint()
{
    for (int i = 0; i < m_curveLines.drags.size(); i++)
    {
        int index = m_curveLines.drags[i];
        modifyPoint(m_curveLines.points[index]);
    }
    m_curveLines.drags.clear();
    repaint();
}

bool QCurveWidget::modifyPoint(CurvePoint &point)
{
    QTableWidget *table = new QTableWidget();
    table->setRowCount(5);
    table->setColumnCount(2);
    table->verticalHeader()->hide();
    table->horizontalHeader()->hide();

    table->setItem(0, 0, new QTableWidgetItem("type"));
    table->setItem(1, 0, new QTableWidgetItem("pos x"));
    table->setItem(2, 0, new QTableWidgetItem("pos y"));
    table->setItem(3, 0, new QTableWidgetItem("aid x"));
    table->setItem(4, 0, new QTableWidgetItem("aid y"));

    table->setItem(0, 1, new QTableWidgetItem(QString::number(point.type)));
    table->setItem(1, 1, new QTableWidgetItem(QString::number(point.pos.x())));
    table->setItem(2, 1, new QTableWidgetItem(QString::number(point.pos.y())));
    table->setItem(3, 1, new QTableWidgetItem(QString::number(point.aid.x())));
    table->setItem(4, 1, new QTableWidgetItem(QString::number(point.aid.y())));

    QDialog dialog;
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.setLayout(new QVBoxLayout());
    dialog.layout()->addWidget(table);
    dialog.layout()->addWidget(&buttons);
    if(dialog.exec() == QDialog::Accepted)
    {
        point.type = (PointType)table->item(0, 1)->text().toInt();
        point.pos.setX(table->item(1, 1)->text().toFloat());
        point.pos.setY(table->item(2, 1)->text().toFloat());
        point.aid.setX(table->item(3, 1)->text().toFloat());
        point.aid.setY(table->item(4, 1)->text().toFloat());
        delete  table;
        return true;
    }
    delete  table;
    return false;
}

QPoint QCurveWidget::toCanvasCoordinates(const QVector2D& pos)
{
    float x = pos.x() * m_scale + m_centerOffset.x();
    float y = -pos.y() * m_scale + m_centerOffset.y();
    return QPoint(static_cast<int>(x), static_cast<int>(y));
}

QVector2D QCurveWidget::toAnalyticCoordinates(const QPoint& pos)
{
    return QVector2D((pos.x() - m_centerOffset.x()) / m_scale, (-pos.y() + m_centerOffset.y()) / m_scale);
}

