#ifndef CIRCLE_CPP
#define CIRCLE_CPP

#include "circle.h"
#include <QPainter>
#include <QDebug>

circle::circle(const QPoint center, const int radius)
    : figure()
{
    points.push_back(center);
    this->radius = radius;
}

void circle::draw(QPainter *painter) {
    qInfo() << "draw circle";
    painter->drawEllipse(points[0], radius, radius);
}

#endif // CIRCLE_CPP
