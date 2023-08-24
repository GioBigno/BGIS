#ifndef SCENE_H
#define SCENE_H

#include <QQuickItem>

class Scene : public QQuickItem{

    Q_OBJECT

public:
    Scene(QQuickItem *parent = 0);

protected:

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;


private:

    QVector<QVector<QPoint>> regions;

    float rotationFactor;
};



#endif // SCENE_H
