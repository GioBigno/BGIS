#ifndef SCENE_H
#define SCENE_H

#include <QtQuick/QQuickPaintedItem>
#include <QFile>
#include "Shape.h"

class Scene : public QQuickPaintedItem{

    Q_OBJECT

public:
    Scene(QQuickItem *parent = 0);

    Q_INVOKABLE void selectedFile(QString filePath);

protected:
    void paint(QPainter *painter) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void readShapeFile(QString fileName);

    void resetMatrix();
    void computeMatrix();

    QPointF screenCenter;
    QPointF worldCenter;

    double scaleFactor;
    double rotationFactor;

    QTransform worldToScreen;
    QTransform screenToWorld;
    QTransform tempMovingMatrix;

    QPointF mouseDragStart;
    bool tempMoving;

    QFile shapeFile;
    QVector<Shape> shapes;
};

#endif // SCENE_H
