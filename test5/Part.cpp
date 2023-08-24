#include <QPainterPath>
#include <QDebug>
#include "Part.h"

Part::Part(const ShapeType t, const QVector<QPointF> &vertices){
    this->t = t;
    this->vertices = vertices;
}

Part::Part(){
    this->t = Point;
    this->vertices = QVector<QPointF>();
}

ShapeType Part::getType(){
    return t;
}

void Part::setType(ShapeType t){
    this->t = t;
}

QVector<QPointF>& Part::getVertices(){
    return vertices;
}

void Part::paint(QPainter *painter){

    QPainterPath pp;

    switch(t){
    case Point:
        for(QPointF &p : vertices){
                painter->drawEllipse(p, 1, 1);
        }

        break;
    case Polylines:
            painter->drawPolyline(vertices.data(), vertices.size());

        break;
    case Polygons:
        //painter->drawPolygon(QPolygonF(vertices));
        pp.addPolygon(QPolygonF(vertices));
        painter->drawPath(pp);
        qDebug() << painter->pen().width();
        //painter->drawPolyline(vertices);
        break;
    case MultiPoint:
        for(QPointF &p : vertices){
                painter->drawEllipse(p, 1, 1);
        }

        break;
    }

}
