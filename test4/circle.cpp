#ifndef CIRCLE_CPP
#define CIRCLE_CPP

#include "circle.h"
#include <QPainter>
#include <QDebug>

circle::circle(QPoint center, int radius){

    points.push_back(center);
    this->radius = radius;
}

void circle::draw(QPainter *painter) const{
    painter->drawEllipse(points[0], radius, radius);
}

#endif // CIRCLE_CPP
