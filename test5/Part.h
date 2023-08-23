#ifndef PART_H
#define PART_H

#include <QPointF>
#include <QVector>
#include <QPainter>

enum ShapeType {
    Point = 1,
    Polylines = 3,
    Polygons = 5,
    MultiPoint = 8
};

class Part{
public:
    Part(const ShapeType t, const QVector<QPointF> &vertices);
    Part();

    ShapeType getType();
    void setType(ShapeType t);
    QVector<QPointF>& getVertices();

    void paint(QPainter *painter);

private:
    ShapeType t;
    QVector<QPointF> vertices;
};


#endif // PART_H
