#ifndef MYHISTRECT_H
#define MYHISTRECT_H

#include <QGraphicsRectItem>

class myHistRect : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    myHistRect();
    myHistRect(qreal x, qreal y, qreal w, qreal h, QGraphicsItem *parent = 0);

    void mousePressEvent( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent * event );

signals:
    void mousePress(QGraphicsSceneMouseEvent *);
    void mouseRelease(QGraphicsSceneMouseEvent *);
    void mouseMove(QGraphicsSceneMouseEvent *);

public slots:

};

#endif // MYHISTRECT_H
