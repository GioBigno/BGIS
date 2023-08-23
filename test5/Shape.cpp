#include "Shape.h"

Shape::Shape(const ShapeType t, const QVector<Part> &parts){

    this->t = t;
    this->parts = parts;
}

ShapeType Shape::getType(){
    return t;
}

QVector<Part>& Shape::getParts(){
    return parts;
}

void Shape::paint(QPainter *painter){

    for(Part &p : parts)
        p.paint(painter);
}
