#include "myhistrect.h"

myHistRect::myHistRect()
{
}

myHistRect::myHistRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent) :
        QGraphicsRectItem(x, y, w, h, parent)
{
}

void myHistRect::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    emit mousePress(event);
}

void myHistRect::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
    emit mouseRelease(event);
}

void myHistRect::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
    emit mouseMove(event);
}
