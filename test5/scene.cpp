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

    tempMoving = 0;
    scaleFactor = 1;

    worldCenter.setX(0);
    worldCenter.setY(0);

    /*
    polygons.push_back(QPolygonF(QVector<QPointF> {
                                    QPointF(100, 100),
                                    QPointF(100, 180),
                                    QPointF(20, 180),
                                    QPointF(20, 100) }
                                 ));

    polygons.push_back(QPolygonF(QVector<QPointF> {
                                    QPointF(0, -60),
                                    QPointF(70, -30),
                                    QPointF(70, 30),
                                    QPointF(0, 60),
                                    QPointF(-70, 30),
                                    QPointF(-70, -30)}
                                 ));
    */

}

void Scene::selectedFile(QString filePath){

    if(filePath.isEmpty()){
        return;
    }else{
        filePath = filePath.remove(0, 5);
    }

    shapeFile.setFileName(filePath);
    if(shapeFile.open(QIODevice::ReadOnly)) {
        qDebug() << "aperto" << filePath;
    }

    readShapeFile(shapeFile.fileName());
    /*
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
*/

    scaleFactor = 1;
    rotationFactor = 0;

    computeMatrix();
    update();

    shapeFile.close();
}

void Scene::paint(QPainter *painter){

    QBrush br(QColor("green"));
    painter->setBrush(br);

    //disegno una x al centro dello schermo
    painter->drawLine(QPointF(screenCenter.rx()-5, screenCenter.ry()-5),
                      QPointF(screenCenter.rx()+5, screenCenter.ry()+5));
    painter->drawLine(QPointF(screenCenter.rx()-5, screenCenter.ry()+5),
                      QPointF(screenCenter.rx()+5, screenCenter.ry()-5));

    painter->setRenderHint(QPainter::Antialiasing);

    painter->resetTransform();

    if(tempMoving){
        painter->setTransform(tempMovingMatrix);
    }

    painter->setTransform(worldToScreen, true);

    for(Shape &s : shapes){
        s.paint(painter);
    }
}

void Scene::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry){
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);

    screenCenter = newGeometry.center();

    computeMatrix();
    update();
}

void Scene::mousePressEvent(QMouseEvent *event){
    QQuickItem::mousePressEvent(event);
    event->accept();

    mouseDragStart = event->position();
    tempMoving = true;
    qDebug() << "click";

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

    if(event->angleDelta().y() > 0){
        scaleFactor *= 1.3;
    }else{
        scaleFactor /= 1.3;
    }

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
        rotationFactor += 90;
    else if(event->key() == Qt::Key_Left)
        rotationFactor -= 90;

    computeMatrix();
    update();
}

void Scene::readShapeFile(QString fileName){

    SHPHandle handle =  SHPOpen(fileName.toStdString().c_str(), "rb");

    shapes.clear();

    int numShape=0;
    int typeShape=-1;
    double a[4], b[4];
    double maxX=0, maxY=0;
    double minX=0, minY=0;

    SHPGetInfo(handle, &numShape, &typeShape, a, b);

    qDebug()<<"[INFO]\n";
    qDebug()<<"numShape: "<<numShape;
    qDebug()<<"type: "<<typeShape;

    qDebug()<<"[READ]\n";

    SHPObject ob;
    for(int i=0; i<numShape; i++){
        ob = *(SHPReadObject(handle, i));

        QVector<Part> parts(ob.nParts);

        if(ob.nParts == 0 && ob.nVertices > 0){
            QVector<QPointF> vertices(ob.nVertices);
            for(int vertex=0; vertex<ob.nVertices; vertex++){
                vertices[vertex].setX(ob.padfX[vertex]);
                minX = std::min(minX, ob.padfX[vertex]);
                maxX = std::max(maxX, ob.padfX[vertex]);
            }

            for(int vertex=0; vertex<ob.nVertices; vertex++){
                vertices[vertex].setY(ob.padfY[vertex]);
                minY = std::min(minY, ob.padfY[vertex]);
                maxY = std::max(maxY, ob.padfY[vertex]);
            }

            parts.push_back(Part(ShapeType::Point, vertices));

        }else{

            int part=0;
            for(part=0; part<ob.nParts; part++){

                int endParts=0;
                if(part == ob.nParts-1)
                    endParts = ob.nVertices;
                else
                    endParts = ob.panPartStart[part+1];

                QVector<QPointF> vertices(endParts-ob.panPartStart[part]);

                int indexVertex = 0;

                for(int vertex=ob.panPartStart[part]; vertex<endParts; vertex++){
                    vertices[indexVertex++].setX(ob.padfX[vertex]);
                    minX = std::min(minX, ob.padfX[vertex]);
                    maxX = std::max(maxX, ob.padfX[vertex]);
                }

                indexVertex = 0;
                for(int vertex=ob.panPartStart[part]; vertex<endParts; vertex++){
                    vertices[indexVertex++].setY(ob.padfY[vertex]);
                    minY = std::min(minY, ob.padfY[vertex]);
                    maxY = std::max(maxY, ob.padfY[vertex]);
                }

                parts[part] = Part(static_cast<ShapeType>(ob.panPartType[part]), vertices);
            }
        }

        shapes.push_back(Shape(static_cast<ShapeType>(ob.nSHPType), parts));
    }

    worldCenter = QPointF((minX/2)+(maxX/2), (minY/2)+(maxY/2));
    qDebug() << "world center: "<<worldCenter;

    SHPClose(handle);
}

void Scene::resetMatrix(){
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

    qDebug() << "world center: "<<worldCenter;
    qDebug() << "toScreen: "<<worldToScreen.map(worldCenter);

    screenToWorld.reset();
    screenToWorld.scale(1, -1);
    screenToWorld.rotate(-rotationFactor);
    screenToWorld.scale(1.0/scaleFactor, 1.0/scaleFactor);
    screenToWorld.translate(-delta.x(), -delta.y());
}
