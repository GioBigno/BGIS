#ifndef SCENE_H
#define SCENE_H

#include <QtQuick/QQuickPaintedItem>
#include <QFile>
#include <QColor>
#include "Shape.h"

class Scene : public QQuickPaintedItem{

    Q_OBJECT

public:
    Scene(QQuickItem *parent = 0);

    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged)
    Q_INVOKABLE void selectedFile(QString filePath);

protected:

    QColor fillColor();
    void setFillColor(QColor color);

    void paint(QPainter *painter) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void debugShapeFile();

private:
    void readShapeFile(QString fileName);

    QColor m_fillColor;
    float borderWidth;

    QRectF window;

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
    QPointF lastMousePositionWorld;
    bool tempMoving;

    QFile shapeFile;
    QVector<Shape> shapes;
};

#endif // SCENE_H
