#ifndef SCENE_H
#define SCENE_H

#include <QtQuick/QQuickPaintedItem>
#include <QColor>

class Scene : public QQuickPaintedItem{

    Q_OBJECT

public:
    Scene(QQuickItem *parent = 0);

protected:
    void paint(QPainter *painter) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void computeMatrix();

    QPointF screenCenter;
    QPointF worldCenter;

    double scaleFactor;

    QTransform worldToScreen;
    QTransform screenToWorld;

    QVector<QPointF> points;
};

#endif // SCENE_H
