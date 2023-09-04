#ifndef SCENE_H
#define SCENE_H

#include <QSGFlatColorMaterial>
#include <QQuickItem>
#include <QFile>
#include <geos/geom/Geometry.h>

class Scene : public QQuickItem{

    Q_OBJECT

public:
    Scene(QQuickItem *parent = 0);

    Q_PROPERTY(QColor fillColor READ fillColor WRITE setFillColor NOTIFY fillColorChanged);
    Q_INVOKABLE void selectedFile(QString filePath);

signals:
    void fillColorChanged();

protected:

    QColor fillColor();
    void setFillColor(QColor color);

    QSGNode* createShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                             const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                             const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial)const;
    void createSceneGraph(QSGNode *worldNode);
    void updateSceneGraph(QSGNode *worldNode);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:

    void debugGeometries();
    void readShapeFile(QString fileName);

    QColor m_fillColor;
    std::unique_ptr<QSGFlatColorMaterial> fillMaterial;
    std::unique_ptr<QSGFlatColorMaterial> borderMaterial;

    void resetMatrix();
    void computeMatrix();

    QTransform screenToWorld;
    QTransform worldToScreen;
    QTransform tempMovingMatrix;

    QPointF mouseDragStart;
    QPointF lastMousePositionWorld;
    bool tempMoving;
    bool createShapeSceneGraph;
    bool updateShapeSceneGraph;

    QPointF screenCenter;
    QPointF worldCenter;

    QRectF window;

    double rotationFactor;
    double scaleFactor;

    QFile shapeFile;

    std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;
    std::vector<size_t> selectedShapes;
};



#endif // SCENE_H
