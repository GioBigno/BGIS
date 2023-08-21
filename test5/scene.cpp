#include "scene.h"
#include <QPainter>
#include <QDebug>

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

    figures.push_back(QPointF(10, 10));

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

    for(QPolygonF &p : polygons){
        painter->drawPolygon(p);
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

    tempMovingMatrix.reset();
    QPointF delta(event->position() - mouseDragStart);

    tempMovingMatrix.translate(delta.x(), delta.y());

    update();
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

void Scene::computeMatrix(){

    worldToScreen.reset();
    worldToScreen.scale(scaleFactor, scaleFactor);
    worldToScreen.rotate(rotationFactor);

    QPointF transformedWorldCenter = worldToScreen.map(worldCenter);
    QPointF delta(screenCenter - transformedWorldCenter);

    worldToScreen.reset();
    worldToScreen.translate(delta.x(), delta.y());
    worldToScreen.scale(scaleFactor, scaleFactor);
    worldToScreen.rotate(rotationFactor);

    screenToWorld.reset();
    screenToWorld.rotate(-rotationFactor);
    screenToWorld.scale(1.0/scaleFactor, 1.0/scaleFactor);
    screenToWorld.translate(-delta.x(), -delta.y());
}
