#ifndef QCURVEEDITWIDGET_H
#define QCURVEEDITWIDGET_H

#include <QWidget>
#include <QVector2D>
#include <QTimer>
#include <QDebug>
#include "curvelines.h"

class QCurveEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QCurveEditWidget(QWidget *parent = nullptr);
    ~QCurveEditWidget() override;

public:
    CurveLines *getCurveLines();
    void addCurveLine(const CurvePoint& point);
    void clearCurveLines();

signals:
    void closeWidget();

public slots:
    void resetView();
    void remoteView();
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

protected slots:
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
