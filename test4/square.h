#ifndef SQUARE_H
#define SQUARE_H

#include "figure.h"

class square : public figure{
public:
    square(const QPoint center, const int side);
    void draw(QPainter *painter) override;
};

#endif // SQUARE_H
