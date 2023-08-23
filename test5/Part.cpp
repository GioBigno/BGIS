#include "Part.h"

Part::Part(const ShapeType t, const QVector<QPointF> &vertices){
    this->t = t;
    this->vertices = vertices;
}

Part::Part(){
    this->t = Point;
    this->vertices = {};
}

ShapeType Part::getType(){
    return t;
}

QVector<QPointF>& Part::getVertices(){
    return vertices;
}

void Part::paint(QPainter *painter){

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
        painter->drawPolygon(QPolygonF(vertices));

        break;
    case MultiPoint:
        for(QPointF &p : vertices){
                painter->drawEllipse(p, 1, 1);
        }

        break;
    }

}
