#ifndef SCENE_H
#define SCENE_H

#include <QSGFlatColorMaterial>
#include <QQuickItem>
#include <QFile>

#include <geos/index/strtree/SimpleSTRtree.h>
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

    QSGNode* createPolygonShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                    const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                    const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial)const;
    QSGNode* createMultiPointShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                       const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                       const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial) const;
    QSGNode* createLineStringShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                       const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                       const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial) const;
    void createSceneGraph(QSGNode *worldNode);
    void updateColorSceneGraph(QSGNode *worldNode);
    void updateSelectionSceneGraph(QSGNode *worldNode);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

    void createSpatialIndex();
    void selectShape(const QPoint &click);
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:

#ifdef DEBUG_BUILD
    void debugGeometries();
#endif
    void readShapeFile(QString fileName);

    QColor m_fillColor;
    std::unique_ptr<QSGFlatColorMaterial> fillMaterialRealShape;
    std::unique_ptr<QSGFlatColorMaterial> borderMaterialRealShape;
    std::unique_ptr<QSGFlatColorMaterial> fillMaterialSelectedShape;
    std::unique_ptr<QSGFlatColorMaterial> borderMaterialSelectedShape;

    void resetMatrix();
    void computeMatrix();

    QTransform screenToWorld;
    QTransform worldToScreen;
    QTransform tempMovingMatrix;

    QPoint mouseDragStart;
    bool tempMoving;
    bool geometriesLoaded;
    bool createShapeSceneGraph;
    bool updateColor;
    bool updateSelection;

    QPointF screenCenter;
    QPointF worldCenter;

    QRectF window;

    double rotationFactor;
    double scaleFactor;

    QFile shapeFile;

    std::vector<std::unique_ptr<geos::geom::Geometry>> geometries;
    std::set<size_t> selectedShapes;
    std::set<size_t> toSelectShape;

    std::unique_ptr<geos::index::strtree::SimpleSTRtree> spatialIndex;
};

#endif // SCENE_H
