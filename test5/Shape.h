#ifndef SHAPE_H
#define SHAPE_H

#include <QPointF>
#include <QVector>
#include <QPainter>
#include "Part.h"

class Shape{

public:
    Shape(const ShapeType t, const QVector<Part> &parts);

    ShapeType getType();
    QVector<Part>& getParts();

    void paint(QPainter *painter);

private:

    ShapeType t;
    QVector<Part> parts;
};

#endif // SHAPE_H
