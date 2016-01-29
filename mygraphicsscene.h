#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "myimageviewer.h"
#include "myhistrect.h"

#define BITS 65535

class myGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit myGraphicsScene(QObject *parent = 0);
    explicit myGraphicsScene(uint initWidth, uint initHeight);
    //~myGraphicsScene();
    void mousePressEvent( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent * event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent * event );

    myImageViewer *imageLabel;
    myImageViewer *lineLabelLow;
    myImageViewer *lineLabelHigh;
    myHistRect *lineLow, *lineHigh;

    QGraphicsLineItem *contrastLine;

    bool histMoveLeft, histMoveRight;

    int width, height;

signals:
    void mousePress(QGraphicsSceneMouseEvent *);
    void mouseRelease(QGraphicsSceneMouseEvent *);
    void mouseMove(QGraphicsSceneMouseEvent *);
    void changeHist();
    void changeHistOutput(int x1, int x2);

public slots:

    void histMousePress(QGraphicsSceneMouseEvent * event);
    void histMouseRelease(QGraphicsSceneMouseEvent * event);
    void histMouseMove(QGraphicsSceneMouseEvent * event);
    void onHistChanged();
    void onHistCalculated(ushort *a);


};

#endif // MYGRAPHICSSCENE_H
