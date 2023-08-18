#include "scene.h"
#include <QPainter>
#include <QDebug>

Scene::Scene(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    scaleFactor = 1;

    worldCenter.setX(0);
    worldCenter.setY(0);

    points.push_back(QPointF(0, 0));
    points.push_back(QPointF(80, 0));
    points.push_back(QPointF(80, 80));
    points.push_back(QPointF(0, 80));
}

void Scene::wheelEvent(QWheelEvent *event){

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

void Scene::paint(QPainter *painter){

    QBrush br(QColor("green"));
    painter->setBrush(br);

    //disegno una x al centro dello schermo
    painter->drawLine(QPointF(screenCenter.rx()-5, screenCenter.ry()-5),
                      QPointF(screenCenter.rx()+5, screenCenter.ry()+5));
    painter->drawLine(QPointF(screenCenter.rx()-5, screenCenter.ry()+5),
                      QPointF(screenCenter.rx()+5, screenCenter.ry()-5));

    painter->setTransform(worldToScreen);

    QPolygonF po(points);
    painter->drawPolygon(po);
}

void Scene::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry){
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);

    screenCenter = newGeometry.center();

    computeMatrix();
    update();
}

void Scene::mousePressEvent(QMouseEvent *event){
    event->accept();
}

void Scene::mouseReleaseEvent(QMouseEvent *event){
    QQuickItem::mouseReleaseEvent(event);

    QPointF pointClick(event->position().x(),
                       event->position().y());

    worldCenter = screenToWorld.map(pointClick);

    computeMatrix();
    update();
}

void Scene::computeMatrix(){

    worldToScreen.reset();
    worldToScreen.scale(scaleFactor, scaleFactor);

    QPointF transformedWorldCenter = worldToScreen.map(worldCenter);

    QPointF delta(screenCenter - transformedWorldCenter);

    worldToScreen.reset();
    worldToScreen.translate(delta.x(), delta.y());
    worldToScreen.scale(scaleFactor, scaleFactor);

    screenToWorld.reset();
    screenToWorld.scale(1.0/scaleFactor, 1.0/scaleFactor);
    screenToWorld.translate(-delta.x(), -delta.y());
}
