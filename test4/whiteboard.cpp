#include "whiteboard.h"
#include <QPainter>
#include <QDebug>
#include "circle.h"

Whiteboard::Whiteboard(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void Whiteboard::paint(QPainter *painter){

    QBrush br(QColor("green"));
    painter->setBrush(br);

    for(figure p : toDraw){
        p.draw(painter);
    }

}

QString Whiteboard::pen() const{

    return m_pen;
}

void Whiteboard::setPen(const QString &pen){
    m_pen = pen;
    qInfo() << "pen changed: " << m_pen;
    update();
}

void Whiteboard::drawPoint(int x, int y){
    if(m_pen == "circle"){
        toDraw.push_back(circle(QPoint(x, y), 5));
    }
    update();
}
