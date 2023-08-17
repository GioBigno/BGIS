#ifndef FIGURE_H
#define FIGURE_H

#include <QtQuick>

class figure{
public:
    void virtual draw(QPainter *painter) const;
protected:
    QVector<QPoint> points;
};

#endif // FIGURE_H
