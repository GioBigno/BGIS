#include "Shape.h"

Shape::Shape(const ShapeType t, const QVector<Part> &parts){

    this->t = t;
    this->parts = parts;
}

Shape::Shape(){
    this->t = ShapeType::Point;
    this->parts = QVector<Part>();
}

ShapeType Shape::getType(){
    return t;
}

void Shape::setType(ShapeType t){
    this->t = t;
}

QVector<Part>& Shape::getParts(){
    return parts;
}

ShapeType Shape::getTypeByInt(int typeInt){

    if(typeInt == 1 || typeInt == 11 || typeInt == 21)
        return ShapeType::Point;
    else if(typeInt == 3 || typeInt == 13 || typeInt == 23)
        return ShapeType::Polylines;
    else if(typeInt == 5 || typeInt == 15 || typeInt == 25)
        return ShapeType::Polygons;
    else if(typeInt == 8 || typeInt == 18 || typeInt == 28)
        return ShapeType::MultiPoint;

    return ShapeType::Point;
}

/*
void Shape::paint(QPainter *painter){

    for(Part &p : parts)
        p.paint(painter);
}
*/
