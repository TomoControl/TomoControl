#include "mygraphicsscene.h"

myGraphicsScene::myGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
}

myGraphicsScene::myGraphicsScene(uint initWidth, uint initHeight)
{
    height = initHeight;
    width = initWidth;

    this->setSceneRect(QRect(0,0,width,height));

    lineLow = new myHistRect;
    lineLow->setRect(-1,-1,2,height);
    lineLow->setPen(QPen(QColor(255,0,0,255), 2));
    lineLow->setBrush(QColor(255,0,0,50));
    this->addItem(lineLow);

    lineHigh = new myHistRect;
    lineHigh->setRect(-1,0,2,height);
    lineHigh->setX(width-3);
    lineHigh->setPen(QPen(QColor(255,0,0,255), 2));
    lineHigh->setBrush(QColor(255,0,0,50));
    this->addItem(lineHigh);

    contrastLine = new QGraphicsLineItem;
    contrastLine->setPen(QPen(QColor(255,0,0,50), 1));
    contrastLine->setLine(lineLow->rect().width(),height,lineHigh->x(),0);
    this->addItem(contrastLine);

    histMoveLeft = 0;
    histMoveRight = 0;

    connect(this, SIGNAL(mousePress(QGraphicsSceneMouseEvent*)), this, SLOT(histMousePress(QGraphicsSceneMouseEvent*)));
    connect(this, SIGNAL(mouseRelease(QGraphicsSceneMouseEvent*)), this, SLOT(histMouseRelease(QGraphicsSceneMouseEvent*)));
    connect(this, SIGNAL(mouseMove(QGraphicsSceneMouseEvent*)), this, SLOT(histMouseMove(QGraphicsSceneMouseEvent*)));
    connect(this, SIGNAL(changeHist()), this, SLOT(onHistChanged()));
}

// переопределение событий изменения состояния мыши
void myGraphicsScene::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    emit mousePress(event);
}

void myGraphicsScene::mouseReleaseEvent( QGraphicsSceneMouseEvent * event )
{
    emit mouseRelease(event);
}

void myGraphicsScene::mouseMoveEvent( QGraphicsSceneMouseEvent * event )
{
    emit mouseMove(event);
}

void myGraphicsScene::histMousePress(QGraphicsSceneMouseEvent * event)
{
    if (event->button() & Qt::LeftButton)
    {
        int position = lineLow->pos().x() + lineLow->rect().width();
        if ((event->scenePos().x()> (position-5))&&(event->scenePos().x() < (position+5)))
        {
            histMoveLeft = 1;
            return;
        }
        if ((event->scenePos().x()> (lineHigh->pos().x()-5))&&(event->scenePos().x() < (lineHigh->pos().x()+5)))
        {
            histMoveRight = 1;
        }
    }
}

void myGraphicsScene::histMouseRelease(QGraphicsSceneMouseEvent * event)
{
    if (event->button() & Qt::LeftButton)
    {
        if (histMoveLeft)
        {
            histMoveLeft = 0;
        }
        if (histMoveRight)
        {
            histMoveRight = 0;
        }
    }
}

// изменение гистограммы по событию движения мыши
void myGraphicsScene::histMouseMove(QGraphicsSceneMouseEvent * event)
{
    if (event->buttons().testFlag(Qt::LeftButton))
    {
        if (histMoveLeft)
        {
            if (((event->scenePos().x()+1) < lineHigh->scenePos().x())&&(event->scenePos().x()>2))
            {
                lineLow->setRect(-1,-1,event->scenePos().x(),height);
                emit changeHist();
                return;
            }
            if (event->scenePos().x()<=2)
            {
                lineLow->setRect(-1,-1,2,height);
                emit changeHist();
            }
            else
            {
                lineLow->setRect(-1,-1,lineHigh->scenePos().x()-2,height);
                emit changeHist();
            }
        }
        if (histMoveRight)
        {
            int tmpWidth;
            if (((event->scenePos().x()-3) > lineLow->scenePos().x() + lineLow->rect().width())&&
                (event->scenePos().x() <= width-3))
            {
                tmpWidth = width - event->scenePos().x();
                lineHigh->setRect(-1,-1,tmpWidth,height);
                lineHigh->setX(event->scenePos().x());
                emit changeHist();
                return;
            }
            if (event->scenePos().x() >= width-3)
            {
                lineHigh->setRect(-1,0,2,height);
                lineHigh->setX(width-3);
                emit changeHist();
            }
            else
            {
                tmpWidth = width - lineLow->rect().width();
                lineHigh->setRect(-1, -1, tmpWidth, height);
                lineHigh->setX(lineLow->rect().width()+2);
                emit changeHist();
            }
        }
    }
}

// Слот обработки события изменения гистограммы
void myGraphicsScene::onHistChanged()
{
    // Изменяем линию преобразования на гистограмме
    contrastLine->setLine(lineLow->rect().width(),height,lineHigh->x(),0);

    // Рассчитываем границы преобразования
    int x1,x2;
    x1 = BITS * lineLow->rect().width()/(width-6);
    x2 = BITS * lineHigh->scenePos().x()/(width-6);

    emit changeHistOutput(x1, x2);
}

void myGraphicsScene::onHistCalculated(ushort *a)
{

    qDebug() << "myGraphicsScene::Отрисовываем гистограмму";

    int max = 0, value = 0, median = 0, nSumm = 0;

    // Приводим гистограмму к 256 значениям

    // Длина области постороения гистограммы
    const ushort histLength = 256;
    // Массив приведенной гистограммы
    ushort b[histLength];
    // Размер усредняемой области для исходной гистограммы
    nSumm = (BITS+1) / histLength;

    // Пересчитываем гистограмму
    for (int k = 0; k < histLength; k++)
    {
        median = 0;
        for (int l = 0; l < nSumm; l++)
        {
            median += a[ k * nSumm + l];
        }
        median = median / nSumm;
        b[k] = median;
    }

    // Вычисляем максимальное значение
    for (int k = 0; k < histLength; k++)
    {
        if (b[k] > max) {max = b[k];}
    }

    qDebug() << "myGraphicsScene::Максимальное значение" << max;

    // Выводим на экран
    for (int k = 0; k < histLength; k++)
    {
        value = height - (b[k] * height / max);
        this->addLine(k, value, k, height);
    }
}


//myGraphicsScene::~myGraphicsScene()
//{
//    disconnect(this, SIGNAL(mousePress(QGraphicsSceneMouseEvent*)), this, SLOT(histMousePress(QGraphicsSceneMouseEvent*)));
//    disconnect(this, SIGNAL(mouseRelease(QGraphicsSceneMouseEvent*)), this, SLOT(histMouseRelease(QGraphicsSceneMouseEvent*)));
//    disconnect(this, SIGNAL(mouseMove(QGraphicsSceneMouseEvent*)), this, SLOT(histMouseMove(QGraphicsSceneMouseEvent*)));
//    disconnect(this, SIGNAL(changeHist()), this, SLOT(onHistChanged()));

//    delete lineLow , lineHigh , contrastLine;
//}
