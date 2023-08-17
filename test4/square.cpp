#include "square.h"
#include <QPainter>
#include <QDebug>

square::square(const QPoint center, const int side)
    : figure()
{
    QPoint topLeft(center.x()-side/2, center.y()-side/2);
    QPoint bottomRight(center.x()+side/2, center.y()+side/2);
    points.push_back(topLeft);
    points.push_back(bottomRight);
}

void square::draw(QPainter *painter) {
    qInfo() << "draw square";
    painter->fillRect(QRectF(points[0], points[1]), painter->brush());

}
