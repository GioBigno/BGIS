#include "whiteboard.h"
#include <QPainter>
#include <QDebug>
#include "circle.h"
#include "square.h"

Whiteboard::Whiteboard(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void Whiteboard::paint(QPainter *painter){

    QBrush br(QColor("green"));
    painter->setBrush(br);


    for(figure *p : toDraw){
        p->draw(painter);
    }


    /*
    for(int i=0; i<toDraw.size(); i++)
        if(dynamic_cast<circle*>(toDraw[i]))
            dynamic_cast<circle*>(toDraw[i])->draw(painter);
    */

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
        toDraw.push_back(new circle(QPoint(x, y), 10));
    }else if(m_pen == "square"){
        toDraw.push_back(new square(QPoint(x, y), 20));
    }
    update();
}
