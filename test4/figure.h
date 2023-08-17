#ifndef FIGURE_H
#define FIGURE_H

#include <QtQuick>

class figure{
public:
    virtual void draw(QPainter *painter) = 0;
    QVector<QPoint> points;
};

#endif // FIGURE_H
