#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <memory>
#include <shapefil.h>
#include "shpreader.h"
#include "Scene.h"

Scene::Scene(QQuickItem *parent)
    :QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_fillColor = QColor("blue");

    lastMousePositionWorld = {0, 0};
    tempMoving = false;
    rotationFactor = 0.1;
    scaleFactor = 5;
    updateShapeSceneGraph = false;

    /*
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

    worldCenter = QPointF(10, 10);
    */
}

QColor Scene::fillColor(){
    return m_fillColor;
}
void Scene::setFillColor(QColor color){
    m_fillColor = color;
    update();
}

void Scene::selectedFile(QString filePath){

    if(filePath.isEmpty()){
        return;
    }else{
        filePath = filePath.remove(0, 5);
    }

    shapeFile.setFileName(filePath);
    if(!shapeFile.open(QIODevice::ReadOnly)) {
        qDebug() << "errore apertura file: " << filePath;
        return;
    }

    resetMatrix();
    readShapeFile(shapeFile.fileName());
    debugGeometries();
    debugGeometries();

    computeMatrix();
    update();

    shapeFile.close();
}

/*

       parent
          |
     tempMovingNode
          |
    __worldNode__
    |           |
    |           |
 _shape_      shape
|      |
|      |
part  part
*/

QSGNode* Scene::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data){

    qDebug() << "update";

    QSGNode *parent = static_cast<QSGNode*>(oldNode);

    if (!parent) {

        parent = new QSGNode;
        parent->setFlag(QSGNode::OwnedByParent, true);

        QSGTransformNode *worldNode = new QSGTransformNode;
        worldNode->setFlag(QSGNode::OwnedByParent, true);
        worldNode->setMatrix(worldToScreen);

        QSGTransformNode *tempMovingNode = new QSGTransformNode;
        tempMovingNode->setFlag(QSGNode::OwnedByParent, true);
        tempMovingNode->setMatrix(tempMovingMatrix);
        tempMovingNode->appendChildNode(worldNode);

        parent->appendChildNode(tempMovingNode);
    }

    QSGTransformNode *tempMovingNode = static_cast<QSGTransformNode*>(parent->firstChild());
    QSGTransformNode *worldNode = static_cast<QSGTransformNode*>(tempMovingNode->firstChild());
/*
    if(updateShapeSceneGraph){
        for(Shape currentShape : shapes){

            QSGNode *currentShapeNode = new QSGNode;

            for(Part currentPart : currentShape.getParts()){

                QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
                currentPartNode->setFlag(QSGNode::OwnedByParent, true);


                QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), currentPart.getVertices().size());
                geometry->setDrawingMode(geometry->DrawLineStrip);
                geometry->setLineWidth(5);

                QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

                for(size_t i=0; i<currentPart.getVertices().size(); i++){
                    points[i].set(currentPart.getVertices()[i].x(), currentPart.getVertices()[i].y());
                }

                currentPartNode->setGeometry(geometry);
                currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

                QSGFlatColorMaterial *material = new QSGFlatColorMaterial;
                material->setColor(QColor("green"));
                currentPartNode->setMaterial(material);
                currentPartNode->setFlag(QSGNode::OwnsMaterial, true);

                currentShapeNode->appendChildNode(currentPartNode);
            }

            worldNode->appendChildNode(currentShapeNode);
        }
        updateShapeSceneGraph = false;
    }
*/

    if(tempMoving)
        tempMovingNode->setMatrix(tempMovingMatrix);
    else
        tempMovingNode->setMatrix(QTransform());

    worldNode->setMatrix(worldToScreen);

    //QMatrix4x4 temp_mat(worldToScreen);
    //temp_mat.translate(width()/2, height()/2);
    //temp_mat.scale(scaleFactor);

    return parent;
}

void Scene::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry){
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    screenCenter = newGeometry.center();
    window = newGeometry;

    computeMatrix();
    update();
}

void Scene::mousePressEvent(QMouseEvent *event){
    QQuickItem::mousePressEvent(event);
    event->accept();
    lastMousePositionWorld = screenToWorld.map(event->position());
    mouseDragStart = event->position();
    tempMoving = true;
}

void Scene::mouseReleaseEvent(QMouseEvent *event){
    QQuickItem::mouseReleaseEvent(event);
    tempMoving = false;

    QPointF delta(screenToWorld.map(event->position()) - screenToWorld.map(mouseDragStart));
    worldCenter = (worldCenter - delta);
    computeMatrix();
    update();
}

void Scene::mouseMoveEvent(QMouseEvent *event){

    if(tempMoving){
        tempMovingMatrix.reset();
        QPointF delta(event->position() - mouseDragStart);
        tempMovingMatrix.translate(delta.x(), delta.y());
        update();
    }
}

void Scene::wheelEvent(QWheelEvent *event){
    QQuickItem::wheelEvent(event);

    if(event->angleDelta().y() > 0)
        scaleFactor *= 1.3;
    else
        scaleFactor /= 1.3;

    QPointF mouseOnScreen(event->position().x(), event->position().y());
    QPointF expectedWorldCenter = screenToWorld.map(mouseOnScreen);
    computeMatrix();

    QPointF worldUnderMouse = screenToWorld.map(mouseOnScreen);
    worldCenter = worldCenter + (expectedWorldCenter - worldUnderMouse);
    computeMatrix();

    update();
}

void Scene::keyReleaseEvent(QKeyEvent *event){
    QQuickItem::keyReleaseEvent(event);

    qDebug()<<"ratation";

    if(event->key() == Qt::Key_Right)
        rotationFactor += 45;
    else if(event->key() == Qt::Key_Left)
        rotationFactor -= 45;

    computeMatrix();
    update();
}

void Scene::readShapeFile(QString fileName){

    geometries.clear();

    bpp::ShpReader reader;
    std::string openError;
    reader.setFile( fileName.toUtf8().data() );
    bool retval = reader.open(true, false, openError);

    while(retval && reader.next()){

        std::unique_ptr<geos::geom::Geometry> currentGeom(nullptr);

        switch(reader.getGeomType()){

        case bpp::gPoint:
            currentGeom = std::move(reader.readPoint()->clone());
            break;
        case bpp::gMultiPoint:
            currentGeom = std::move(reader.readMultiPoint()->clone());
            break;
        case bpp::gLine:
            currentGeom = std::move(reader.readLineString()->clone());
            break;
        case bpp::gPolygon:
            currentGeom = std::move(reader.readMultiPolygon()->clone());
            break;
        case bpp::gUnknown:
            qDebug() << "[shpReader]: geometry unknow";
            break;
        }

        if(currentGeom)
            geometries.push_back(std::move(currentGeom));

    }


    worldCenter = QPointF((reader.getMinX()/2)+(reader.getMaxX()/2),
                          (reader.getMinY()/2)+(reader.getMaxY()/2));

    double scaleWidth = window.width() / (reader.getMaxX() - reader.getMinX());
    double scaleHeight = window.height() / (reader.getMaxY() - reader.getMinY());

    if(scaleWidth < scaleHeight)
        scaleFactor = scaleWidth;
    else
        scaleFactor = scaleHeight;

    updateShapeSceneGraph = true;
}

void Scene::resetMatrix(){
    scaleFactor = 0;
    rotationFactor = 0;
    worldToScreen.reset();
    screenToWorld.reset();
}

void Scene::computeMatrix(){

    qDebug() << "compute Matrix";

    worldToScreen.reset();
    worldToScreen.scale(scaleFactor, scaleFactor);
    worldToScreen.rotate(rotationFactor);
    worldToScreen.scale(1, -1);

    QPointF transformedWorldCenter = worldToScreen.map(worldCenter);
    QPointF delta(screenCenter - transformedWorldCenter);

    worldToScreen.reset();
    worldToScreen.translate(delta.x(), delta.y());
    worldToScreen.scale(scaleFactor, scaleFactor);
    worldToScreen.rotate(rotationFactor);
    worldToScreen.scale(1, -1);

    screenToWorld.reset();
    screenToWorld.scale(1, -1);
    screenToWorld.rotate(-rotationFactor);
    screenToWorld.scale(1.0/scaleFactor, 1.0/scaleFactor);
    screenToWorld.translate(-delta.x(), -delta.y());
}

void Scene::debugGeometries(){

    qDebug() << "[debug geometries]";

    qDebug() << "size: " << geometries.size();

    for(size_t i=0; i<geometries.size(); i++){

        const std::unique_ptr<geos::geom::Geometry> currentGeometry(std::move(geometries.at(i).get()->clone()));

        qDebug() << "letto";

        qDebug() << "tipo: " << currentGeometry->getGeometryType();
        qDebug() << "n punti: " << currentGeometry->getNumPoints();

        for(size_t iGeom = 0; iGeom < currentGeometry->getNumGeometries(); iGeom++){

            qDebug() << "geometria n." << iGeom;

            if(currentGeometry->getGeometryTypeId() == geos::geom::GEOS_MULTIPOLYGON){

                const geos::geom::Polygon* currentP = dynamic_cast<const geos::geom::Polygon*>(currentGeometry->getGeometryN(iGeom));

                const geos::geom::LinearRing* outRing = currentP->getExteriorRing();

                qDebug() << "exterior ring: ";
                std::unique_ptr<geos::geom::CoordinateSequence> outSeqPtr(outRing->getCoordinates());

                for(size_t iCoord = 0; iCoord < outSeqPtr->getSize(); iCoord++){
                    qDebug() << (outSeqPtr->getAt(iCoord)).toString();
                }

                for(size_t iHoles = 0; iHoles < currentP->getNumInteriorRing(); iHoles++){

                    const geos::geom::LinearRing* inRing = currentP->getInteriorRingN(iHoles);

                    qDebug() << "interior ring: ";
                    std::unique_ptr<geos::geom::CoordinateSequence> inSeqPtr(inRing->getCoordinates());

                    for(size_t iCoord = 0; iCoord < inSeqPtr->getSize(); iCoord++){
                        qDebug() << (inSeqPtr->getAt(iCoord)).toString();
                    }

                }
            }
        }
    }
}
