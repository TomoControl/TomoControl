#ifndef MYIMAGEVIEWER_H
#define MYIMAGEVIEWER_H

#include <QLabel>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QFrame>
#include <QResizeEvent>
#include <QGridLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QDebug>
#include <QTime>

#include "definitions.h"

#define BITS 65535


class myImageViewer : public QLabel
{
    Q_OBJECT
public:
    explicit myImageViewer(QWidget *parent = 0);
    void wheelEvent( QWheelEvent * event );
    void mousePressEvent( QMouseEvent * event );
    void mouseReleaseEvent( QMouseEvent * event );
    void mouseMoveEvent( QMouseEvent * event );

signals:
    void wheel(QWheelEvent *);
    void mousePress(QMouseEvent *);
    void mouseRelease(QMouseEvent *);
    void mouseMove(QMouseEvent *);

public slots:

};

class myFrame : public QFrame
{
    Q_OBJECT
public:
    explicit myFrame(QWidget *parent = 0);
    //~myFrame();
    void resizeEvent( QResizeEvent * event );
    void setRAWImage(ushort * RAWData);

    QImage applyAutoContrast(int x1, int x2);



    QGridLayout * layout;
    QScrollArea * scrollArea;
    myImageViewer * imageLabel;

    QImage rxImage;
    ushort * imageData;
    ushort a[BITS+1];

    float scale;
    int imageX, imageY;

signals:
    void resize(QResizeEvent *);
    void histCalculated(ushort *);
    void finished();

public slots:
    void labelWheel(QWheelEvent * event);
    void labelMousePress(QMouseEvent * event);
    void labelMouseRelease(QMouseEvent * event);
    void labelMouseMove(QMouseEvent * event);
    void scrollAreaResize(QResizeEvent * event);
    void onChangeHistogrammWidget(int x1, int x2);
    void setImage(ushort * RAWData);
};




#endif // MYIMAGEVIEWER_H
