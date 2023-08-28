#include <QQuickItem>
#include <QTimer>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <shapefil.h>
#include "Scene.h"
#include "Shape.h"

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

void Scene::fillColorChanged(){

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

    SHPHandle handle =  SHPOpen(fileName.toStdString().c_str(), "rb");

    shapes.clear();

    int numShape=0;
    double mins[4], maxs[4];

    SHPGetInfo(handle, &numShape, NULL, mins, maxs);

    double minX = mins[0];
    double minY = mins[1];
    double maxX = maxs[0];
    double maxY = maxs[1];

    shapes.resize(numShape);

    SHPObject ob;
    for(int i=0; i<numShape; i++){
        ob = *(SHPReadObject(handle, i));

        shapes[i].setType( Shape::getTypeByInt(ob.nSHPType));

        if(ob.nParts == 0 && ob.nVertices > 0){

            shapes[i].getParts().resize(1);
            shapes[i].getParts()[0].setType(ShapeType::Point);
            shapes[i].getParts()[0].getVertices().resize(ob.nVertices);

            for(int vertex=0; vertex<ob.nVertices; vertex++)
                shapes[i].getParts()[0].getVertices()[vertex].setX(ob.padfX[vertex]);

            for(int vertex=0; vertex<ob.nVertices; vertex++)
                shapes[i].getParts()[0].getVertices()[vertex].setY(ob.padfY[vertex]);

        }else{

            shapes[i].getParts().resize(ob.nParts);

            for(int part=0; part<ob.nParts; part++){

                int endParts=0;
                if(part == ob.nParts-1)
                    endParts = ob.nVertices;
                else
                    endParts = ob.panPartStart[part+1];

                shapes[i].getParts()[part].setType(Shape::getTypeByInt(ob.panPartType[part]));
                shapes[i].getParts()[part].getVertices().resize(endParts - ob.panPartStart[part]);
                //TODO: qesta parte è un buco? aka: il suo vettore di vertici è in seso antiorario?

                int indexVertex = 0;
                for(int vertex=ob.panPartStart[part]; vertex<endParts; vertex++)
                    shapes[i].getParts()[part].getVertices()[indexVertex++].setX(ob.padfX[vertex]);

                indexVertex = 0;
                for(int vertex=ob.panPartStart[part]; vertex<endParts; vertex++)
                    shapes[i].getParts()[part].getVertices()[indexVertex++].setY(ob.padfY[vertex]);
            }
        }
    }

    worldCenter = QPointF((minX/2)+(maxX/2), (minY/2)+(maxY/2));

    double scaleWidth = window.width() / (maxX - minX);
    double scaleHeight = window.height() / (maxY - minY);

    if(scaleWidth < scaleHeight)
        scaleFactor = scaleWidth;
    else
        scaleFactor = scaleHeight;

    SHPClose(handle);

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

void Scene::debugShapeFile(){
    qDebug()<<"DEBUG SHAPES";
    qDebug()<<"numero shapes: " << shapes.size();

    for(Shape &shape : shapes){

        qDebug()<<"##########################################";

        qDebug()<<"tipo shape: "<<shape.getType();

        QVector<Part> parti = shape.getParts();
        qDebug()<<"numero parti:"<<parti.size();

        for(Part &part : parti){

            qDebug()<<"-----------------------------------------------";

            QVector<QPointF> vertici = part.getVertices();

            qDebug()<<"tipo parte: "<<part.getType();
            qDebug()<<"numero di vertici: "<<vertici.size();

            for(QPointF &punto : vertici) {
                qDebug()<<"X: "<<punto.x()<<" Y: "<<punto.y();
            }

            qDebug()<<"--------------------------------------------------";
        }

        qDebug()<<"##########################################";
    }
}
