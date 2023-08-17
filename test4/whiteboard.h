#ifndef WHITEBOARD_H
#define WHITEBOARD_H

#include <QtQuick/QQuickPaintedItem>
#include <QColor>
#include "figure.h"
#include "circle.h"

class Whiteboard : public QQuickPaintedItem{

    Q_OBJECT
    Q_PROPERTY(QString pen READ pen WRITE setPen)

public:
    Whiteboard(QQuickItem *parent = 0);

    QString pen() const;
    void setPen(const QString &pen);

    Q_INVOKABLE void drawPoint(int x, int y);

protected:
    void paint(QPainter *painter) override;

private:
    QString m_pen;
    QVector<figure> toDraw;

};

#endif // WHITEBOARD_H
