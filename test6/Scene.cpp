#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

#include <memory>
#include <cassert>

#include <shapefil.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>
#include <geos/triangulate/tri/TriList.h>

#include "shpreader.h"
#include "Scene.h"

Scene::Scene(QQuickItem *parent)
    :QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_fillColor = QColor("orange");

    tempMoving = false;
    rotationFactor = 0.1;
    scaleFactor = 5;

    createShapeSceneGraph = false;
    updateColor = false;
    updateSelection = false;

    worldCenter = QPointF(10, 10);
}

QColor Scene::fillColor(){
    return m_fillColor;
}
void Scene::setFillColor(QColor color){
    m_fillColor = color;
    updateColor = true;
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

    selectedShapes.clear();
    updateSelection = false;

    createSpatialIndex();
    computeMatrix();
    update();

    shapeFile.close();
}

QSGNode* Scene::createPolygonShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial) const
{

    QSGNode *currentShapeNode = new QSGNode;
    currentShapeNode->setFlag(QSGNode::OwnedByParent, true);

    // bordi /////////////////////////////////////////////////////////////////////////////////////////////////

    QSGNode *currentEdgeNode = new QSGNode;
    currentEdgeNode->setFlag(QSGNode::OwnedByParent, true);

    for(size_t iPart = 0; iPart < geom->getNumGeometries(); iPart++){

        const geos::geom::Polygon* part = dynamic_cast<const geos::geom::Polygon*>(geom->getGeometryN(iPart));

        for(size_t iRing = 0; iRing < part->getNumInteriorRing() + 1; iRing++){

            const geos::geom::LinearRing* outRing;

            if(iRing == 0)
                outRing = part->getExteriorRing();
            else
                outRing = part->getInteriorRingN(iRing-1);

            QSGGeometryNode *outRingNode  = new QSGGeometryNode;
            outRingNode->setFlag(QSGNode::OwnedByParent, true);

            QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), outRing->getNumPoints());
            geometry->setDrawingMode(geometry->DrawLineStrip);
            geometry->setLineWidth(5);

            QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

            for(size_t i=0; i<outRing->getNumPoints(); i++){
                points[i].set(outRing->getCoordinateN(i).x, outRing->getCoordinateN(i).y);
            }

            outRingNode->setGeometry(geometry);
            outRingNode->setFlag(QSGNode::OwnsGeometry, true);

            outRingNode->setMaterial(borderMaterial.get());

            currentEdgeNode->appendChildNode(outRingNode);
        }
    }
    currentShapeNode->appendChildNode(currentEdgeNode);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////// riempimento ///////////////////////////////////////////////////////////////////////////////////////////

    QSGNode *currentFillingNode = new QSGNode;
    currentFillingNode->setFlag(QSGNode::OwnedByParent, true);

    for(size_t iPart = 0; iPart < geom->getNumGeometries(); iPart++){

        const geos::geom::Polygon* part = dynamic_cast<const geos::geom::Polygon*>(geom->getGeometryN(iPart));

        TriList<Tri> trianglesVertices;
        geos::triangulate::polygon::ConstrainedDelaunayTriangulator::triangulatePolygon(part, trianglesVertices);

        QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
        currentPartNode->setFlag(QSGNode::OwnedByParent, true);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), trianglesVertices.size()*3);
        geometry->setDrawingMode(geometry->DrawTriangleFan);
        geometry->setLineWidth(5);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        int iTriangle = 0;

        for(auto &triangle : trianglesVertices){
            for(size_t iVertex = 0; iVertex < 3; iVertex++){
                points[iTriangle*3+iVertex].set(triangle->getCoordinate(iVertex).x,
                                                triangle->getCoordinate(iVertex).y);
            }
            iTriangle++;
        }

        currentPartNode->setGeometry(geometry);
        currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

        currentPartNode->setMaterial(fillMaterial.get());

        currentFillingNode->appendChildNode(currentPartNode);
    }

    currentShapeNode->appendChildNode(currentFillingNode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return currentShapeNode;
}

QSGNode* Scene::createMultiPointShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                       const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                       const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial) const
{

    QSGNode *currentShapeNode = new QSGNode;
    currentShapeNode->setFlag(QSGNode::OwnedByParent, true);

    // bordi /////////////////////////////////////////////////////////////////////////////////////////////////

    QSGNode *currentEdgeNode = new QSGNode;
    currentEdgeNode->setFlag(QSGNode::OwnedByParent, true);

    for(size_t iPart = 0; iPart < geom->getNumGeometries(); iPart++){

        const geos::geom::Point* part = dynamic_cast<const geos::geom::Point*>(geom->getGeometryN(iPart));

        QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
        currentPartNode->setFlag(QSGNode::OwnedByParent, true);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 1);
        geometry->setDrawingMode(geometry->DrawPoints);
        geometry->setLineWidth(5);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
        points[0].set(part->getCoordinate()->x, part->getCoordinate()->y);

        currentPartNode->setGeometry(geometry);
        currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

        currentPartNode->setMaterial(borderMaterial.get());

        currentEdgeNode->appendChildNode(currentPartNode);
    }

    currentShapeNode->appendChildNode(currentEdgeNode);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////// riempimento ///////////////////////////////////////////////////////////////////////////////////////////

    QSGNode *currentFillingNode = new QSGNode;
    currentFillingNode->setFlag(QSGNode::OwnedByParent, true);

    for(size_t iPart = 0; iPart < geom->getNumGeometries(); iPart++){

        const geos::geom::Point* part = dynamic_cast<const geos::geom::Point*>(geom->getGeometryN(iPart));

        QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
        currentPartNode->setFlag(QSGNode::OwnedByParent, true);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 1);
        geometry->setDrawingMode(geometry->DrawPoints);
        geometry->setLineWidth(5);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();
        points[0].set(part->getCoordinate()->x, part->getCoordinate()->y);

        currentPartNode->setGeometry(geometry);
        currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

        currentPartNode->setMaterial(fillMaterial.get());

        currentEdgeNode->appendChildNode(currentPartNode);
    }

    currentShapeNode->appendChildNode(currentFillingNode);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return currentShapeNode;
}

QSGNode* Scene::createLineStringShapeNode(const std::unique_ptr<geos::geom::Geometry> &geom,
                                          const std::unique_ptr<QSGFlatColorMaterial> &fillMaterial,
                                          const std::unique_ptr<QSGFlatColorMaterial> &borderMaterial) const
{

    QSGNode *currentShapeNode = new QSGNode;
    currentShapeNode->setFlag(QSGNode::OwnedByParent, true);

    // bordi /////////////////////////////////////////////////////////////////////////////////////////////////
    {
        QSGNode *currentEdgeNode = new QSGNode;
        currentEdgeNode->setFlag(QSGNode::OwnedByParent, true);

        const geos::geom::LineString* part = dynamic_cast<const geos::geom::LineString*>(geom.get());

        QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
        currentPartNode->setFlag(QSGNode::OwnedByParent, true);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), part->getNumPoints());
        geometry->setDrawingMode(geometry->DrawLines);
        geometry->setLineWidth(5);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        for(size_t iVertex=0; iVertex<part->getNumPoints(); iVertex++)
            points[iVertex].set(part->getCoordinateN(iVertex).x, part->getCoordinateN(iVertex).y);

        currentPartNode->setGeometry(geometry);
        currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

        currentPartNode->setMaterial(borderMaterial.get());

        currentEdgeNode->appendChildNode(currentPartNode);
        currentShapeNode->appendChildNode(currentEdgeNode);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////// riempimento ///////////////////////////////////////////////////////////////////////////////////////////
    {
        QSGNode *currentFillingNode = new QSGNode;
        currentFillingNode->setFlag(QSGNode::OwnedByParent, true);

        const geos::geom::LineString* part = dynamic_cast<const geos::geom::LineString*>(geom.get());

        QSGGeometryNode *currentPartNode  = new QSGGeometryNode;
        currentPartNode->setFlag(QSGNode::OwnedByParent, true);

        QSGGeometry *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), part->getNumPoints());
        geometry->setDrawingMode(geometry->DrawLines);
        geometry->setLineWidth(5);

        QSGGeometry::Point2D *points = geometry->vertexDataAsPoint2D();

        for(size_t iVertex=0; iVertex<part->getNumPoints(); iVertex++)
            points[iVertex].set(part->getCoordinateN(iVertex).x, part->getCoordinateN(iVertex).y);

        currentPartNode->setGeometry(geometry);
        currentPartNode->setFlag(QSGNode::OwnsGeometry, true);

        currentPartNode->setMaterial(fillMaterial.get());

        currentFillingNode->appendChildNode(currentPartNode);
        currentShapeNode->appendChildNode(currentFillingNode);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return currentShapeNode;
}

void Scene::createSceneGraph(QSGNode *worldNode){

    fillMaterialRealShape->setColor(m_fillColor);
    borderMaterialRealShape->setColor("black");

    QColor selecColor(Qt::lightGray);
    selecColor.setAlpha(150);
    fillMaterialSelectedShape->setColor(selecColor);

    QSGNode *realShapes = new QSGNode;
    realShapes->setFlag(QSGNode::OwnedByParent, true);

    QSGNode *selectedShape = new QSGNode;
    selectedShape->setFlag(QSGNode::OwnedByParent, true);

    for(const std::unique_ptr<geos::geom::Geometry> &shape : geometries){

        QSGNode *currentShapeNode = nullptr;

        switch(shape->getGeometryTypeId()){
        case geos::geom::GEOS_MULTIPOLYGON:
            currentShapeNode = createPolygonShapeNode(shape, fillMaterialRealShape, borderMaterialRealShape);
            break;
        case geos::geom::GEOS_MULTIPOINT:
            currentShapeNode = createMultiPointShapeNode(shape, fillMaterialRealShape, borderMaterialRealShape);
            break;
        case geos::geom::GEOS_POINT:
            currentShapeNode = createMultiPointShapeNode(shape, fillMaterialRealShape, borderMaterialRealShape);
            break;
        case geos::geom::GEOS_LINESTRING:
            currentShapeNode = createLineStringShapeNode(shape, fillMaterialRealShape, borderMaterialRealShape);
            break;
        }

        realShapes->appendChildNode(currentShapeNode);
    }

    worldNode->appendChildNode(realShapes);
    worldNode->appendChildNode(selectedShape);
}

void Scene::updateSelectionSceneGraph(QSGNode *worldNode){

    if(worldNode->childCount() > 0){

        worldNode->removeChildNode(worldNode->lastChild());

        QSGNode *selectedShapeNode = new QSGNode;

        for(const size_t &indexShape : selectedShapes){

            QSGNode *currentShapeNode = createPolygonShapeNode(geometries[indexShape], fillMaterialSelectedShape, borderMaterialSelectedShape);
            selectedShapeNode->appendChildNode(currentShapeNode);
        }

        worldNode->appendChildNode(selectedShapeNode);
    }
}

void Scene::updateColorSceneGraph(QSGNode *worldNode){

    fillMaterialRealShape->setColor(m_fillColor);
}

QSGNode* Scene::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data){

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

        fillMaterialRealShape.reset(new QSGFlatColorMaterial);
        borderMaterialRealShape.reset(new QSGFlatColorMaterial);
        fillMaterialSelectedShape.reset(new QSGFlatColorMaterial);
        borderMaterialSelectedShape.reset(new QSGFlatColorMaterial);
    }

    QSGTransformNode *tempMovingNode = static_cast<QSGTransformNode*>(parent->firstChild());
    QSGTransformNode *worldNode = static_cast<QSGTransformNode*>(tempMovingNode->firstChild());

    if(createShapeSceneGraph){
        //new file opened
        worldNode->removeAllChildNodes();
        createSceneGraph(worldNode);
        createShapeSceneGraph = false;
    }

    if(updateColor){
        updateColorSceneGraph(worldNode);
        updateColor = false;
    }

    if(updateSelection){
        updateSelectionSceneGraph(worldNode);
        updateSelection = false;
    }


    if(tempMoving)
        tempMovingNode->setMatrix(tempMovingMatrix);
    else
        tempMovingNode->setMatrix(QTransform());

    worldNode->setMatrix(worldToScreen);

    return parent;
}

void Scene::createSpatialIndex(){

    //spatialIndex
    /*
    spatialIndex.reset(new geos::index::strtree::SimpleSTRtree);

    for(size_t iGeom=0; iGeom < geometries.size(); iGeom++){

        std::unique_ptr<geos::geom::Envelope> env(new geos::geom::Envelope());

        spatialIndex->insert(geometries[iGeom].get()->getBoundary().get()->getEnvelope(), (void*)iGeom);
    }
*/

}

void Scene::selectShape(const QPoint &click){

    QPoint pointClick = screenToWorld.map(click);

    /*
    std::vector<void *> idk;

    geos::geom::CoordinateXY coord(pointClick.x(), pointClick.y());
    std::unique_ptr<geos::geom::Envelope> env(new geos::geom::Envelope(coord));
    spatialIndex->query(env.get(), idk);

    qDebug() << "idk size: " << idk.size();
    */

    /*
    for(size_t iGeom=0; iGeom < geometries.size(); iGeom++){

        geos::geom::Geometry *shape = geometries[iGeom].get();

        geos::geom::Coordinate coord(pointClick.x(), pointClick.y());
        geos::geom::GeometryFactory::Ptr temp;
        geos::geom::Point *p = temp->createPoint(coord).release();

        if(shape->contains(p)){
            if(selectedShapes.count(iGeom))
                selectedShapes.erase(iGeom);
            else
                selectedShapes.insert(iGeom);

            break;
        }
    }
    */
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
    mouseDragStart = event->position();
    tempMoving = true;
}

void Scene::mouseReleaseEvent(QMouseEvent *event){
    QQuickItem::mouseReleaseEvent(event);
    tempMoving = false;

    if(event->pos() == mouseDragStart){ //click

        updateSelection = true;
        selectShape(event->pos());

    }else{ //drag

        QPointF delta(screenToWorld.map(event->position()) - screenToWorld.map(mouseDragStart));
        worldCenter = (worldCenter - delta);
        computeMatrix();
    }

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

    //qDebug()<<"rotation";

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

    createShapeSceneGraph = true;
}

void Scene::resetMatrix(){
    scaleFactor = 0;
    rotationFactor = 0;
    worldToScreen.reset();
    screenToWorld.reset();
}

void Scene::computeMatrix(){

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
