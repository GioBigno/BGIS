#ifndef CIRCLE_H
#define CIRCLE_H

#include "figure.h"

class circle : public figure{
public:
    circle(const QPoint center, const int radius);
    void draw(QPainter *painter) const;
private:
    int radius;
};

#endif // CIRCLE_H
