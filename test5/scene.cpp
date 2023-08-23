#include <QPainter>
#include <QDebug>
#include <algorithm>
#include "scene.h"
#include <shapefil.h>
#include "Shape.h"

Scene::Scene(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    lastMousePositionWorld = {0, 0};
    tempMoving = false;
    scaleFactor = 1;
    rotationFactor = 0;

    //worldCenter.setX(0);
    //worldCenter.setY(0);
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

void Scene::paint(QPainter *painter){

    QBrush br(QColor("green"));
    painter->setBrush(br);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->resetTransform();

    if(tempMoving)
        painter->setTransform(tempMovingMatrix);

    painter->setTransform(worldToScreen, true);

    for(Shape &s : shapes){
        s.paint(painter);
    }

    painter->resetTransform();
    //disegno una x al centro dello schermo
    painter->setPen(Qt::darkBlue);
    painter->drawLine(QPointF(screenCenter.x(), screenCenter.y()-6),
                      QPointF(screenCenter.x(), screenCenter.y()+6));
    painter->drawLine(QPointF(screenCenter.x()-6, screenCenter.y()),
                      QPointF(screenCenter.x()+6, screenCenter.y()));

    //coordinate(mondo) del mouse
    painter->setPen(Qt::black);
    int fontSize = painter->fontInfo().pixelSize();
    QString text;
    text += "x: ";
    text += std::to_string(lastMousePositionWorld.x());
    text += "  y: ";
    text += std::to_string(lastMousePositionWorld.y());
    painter->drawText(QPoint(10, window.height() - fontSize), text);
}

void Scene::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry){
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);

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
