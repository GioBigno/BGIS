#include <QQuickItem>
#include <QTimer>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include "Scene.h"

Scene::Scene(QQuickItem *parent)
    :QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    rotationFactor = 0.1;

    QVector<QPoint> lombardia = {QPoint(10, 10),
                                QPoint(5, 8),
                                QPoint(-2, -1),
                                QPoint(0, 0)};

    QVector<QPoint> molise = {QPoint(50, 3),
                                QPoint(55, 5),
                                QPoint(52, -2),
                                QPoint(50, 1)};

    regions.push_back(lombardia);
    regions.push_back(molise);

}

QSGNode* Scene::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data){

    qDebug() << "update";

    QSGTransformNode* parent = static_cast<QSGTransformNode*>(oldNode);

    if (!parent) {

        parent = new QSGTransformNode;
        parent->setFlag(QSGNode::OwnedByParent, true);

        for(QVector<QPoint> currentRegion : regions){

            QSGGeometryNode *tempRegionNode  = new QSGGeometryNode;
            tempRegionNode->setFlag(QSGNode::OwnedByParent, true);


            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), currentRegion.size());
            //geometry->setDrawingMode(geometry->DrawLineStrip);
            //geometry->setLineWidth(5);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

            for(size_t i=0; i<currentRegion.size(); i++){
                points[i].set(currentRegion[i].x(), currentRegion[i].y());
            }

            tempRegionNode->setGeometry(geometry);
            tempRegionNode->setFlag(QSGNode::OwnsGeometry, true);

            QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
            material->setColor(QColor("green"));
            tempRegionNode->setMaterial(material);
            tempRegionNode->setFlag(QSGNode::OwnsMaterial, true);


            qDebug("appendo figlio");
            parent->appendChildNode(tempRegionNode);
        }
    }

    QMatrix4x4 temp_mat;
    temp_mat.translate(width()/2, height()/2);
    temp_mat.scale(5);
    parent->setMatrix(temp_mat);



    return parent;

}

