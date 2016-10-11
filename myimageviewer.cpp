#include "myimageviewer.h"

myImageViewer::myImageViewer(QWidget *parent) :
    QLabel(parent)
{
    this->setScaledContents(true);
    this->setBackgroundRole(QPalette::Base);
    this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void myImageViewer::wheelEvent( QWheelEvent * event )
{
    emit wheel(event);
}

void myImageViewer::mousePressEvent( QMouseEvent * event )
{
    emit mousePress(event);
}

void myImageViewer::mouseReleaseEvent( QMouseEvent * event )
{
    emit mouseRelease(event);
}

void myImageViewer::mouseMoveEvent( QMouseEvent * event )
{
    emit mouseMove(event);
}


myFrame::myFrame(QWidget *parent) :
    QFrame(parent)
{
    // Создаем новую метку для вывода изображения
    imageLabel  = new myImageViewer(this);

    // Создаем слой для размещения расширяемой области в данном фрейме
    layout = new QGridLayout;
    layout->setMargin(0);
    this->setLayout(layout);

    // Создаем расширяемую область
    scrollArea = new QScrollArea;
    // Добавляем область на новый слой
    layout->addWidget(scrollArea);
    scrollArea->setBackgroundRole(QPalette::Dark);
    // Добавляем метку для вывода изображения в расширяемую область
    scrollArea->setWidget(imageLabel);

    // Подготовка переменной изображения, выводимого на экран
    QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_Indexed8);
    rxImage = image;

    // Обработка сигналов для событий мыши на изображении
    connect(imageLabel, SIGNAL(wheel(QWheelEvent *)), this, SLOT(labelWheel(QWheelEvent *)));
    connect(imageLabel, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(labelMousePress(QMouseEvent*)));
    connect(imageLabel, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(labelMouseRelease(QMouseEvent*)));
    connect(imageLabel, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(labelMouseMove(QMouseEvent*)));

    // Обработка сигнала изменения размеров окна
    connect(this, SIGNAL(resize(QResizeEvent *)), this, SLOT(scrollAreaResize(QResizeEvent*)));

}

void myFrame::resizeEvent( QResizeEvent * event )
{
    emit resize(event);
}

void myFrame::labelWheel(QWheelEvent * event)
{
    // Указатели на полосы прокрутки
    QScrollBar * hBar, * vBar;
    hBar = scrollArea->horizontalScrollBar();
    vBar = scrollArea->verticalScrollBar();

    // Величина и направление прокрутки колесом мыши
    int delta = event->delta();
    // Координаты события прокрутки
    imageX = event->x();
    imageY = event->y();
    // Значение полос прокрутки
    int barX = hBar->value();
    int barY = vBar->value();
    float oldScale = scale;

    if (delta > 0)
    {
        // Увеличение масштаба
        scale *= (float)1.2;
        if (scale > 5)
            scale = oldScale;
    }
    else
    {
        // Уменьшение масштаба
        scale *= (float)0.8333333333;
        if (scale < ((float)scrollArea->size().width()-2)/((float)rxImage.size().width()))
             scale = oldScale;
    }

    // Изменение размеров метки вывода изображения
    imageLabel->resize(rxImage.size()*scale);

    // Установка скорректированных значений полос прокрутки относительно мыши
    hBar->setValue(imageX*(scale/oldScale) - (imageX - barX));
    vBar->setValue(imageY*(scale/oldScale) - (imageY - barY));
}

void myFrame::labelMousePress(QMouseEvent * event)
{
    // Проверяем нажатую кнопку мыши
    if (event->button() & Qt::MiddleButton)
    {
        // Запоминаем координаты нажатия мыши
        imageX = event->globalX();
        imageY = event->globalY();
        // Изменияем иконку курсора
        setCursor(Qt::OpenHandCursor);
    }
}

void myFrame::labelMouseRelease(QMouseEvent * event)
{
    // Проверяем нажатую кнопку мыши
    if (event->button() & Qt::MiddleButton)
    {
        // Изменияем иконку курсора
        setCursor(Qt::CustomCursor);
    }
}

void myFrame::labelMouseMove(QMouseEvent * event)
{
    // Проверяем нажатую кнопку мыши
    // Для этого события именно так, хз почему
    if (event->buttons().testFlag(Qt::MiddleButton))
    {
        // Указатели на полосы прокрутки
        QScrollBar * hBar, * vBar;
        hBar = scrollArea->horizontalScrollBar();
        vBar = scrollArea->verticalScrollBar();

        // Изменение положения полос прокрутки на изменение координаты мыши
        hBar->setValue(hBar->value()-(event->globalX()-imageX));
        vBar->setValue(vBar->value()-(event->globalY()-imageY));
        // Запоминем новое положение мыши
        imageX = event->globalX();
        imageY = event->globalY();
    }
}

void myFrame::scrollAreaResize(QResizeEvent * event)
{
    // Автоматически меняем масштаб изображения под новый размер окна
    scale = ((float)event->size().width()-2)/((float)rxImage.size().width());
    imageLabel->resize(rxImage.size()*scale);
}

void myFrame::setImage(ushort* raw)
{
    ushort* tmp;
    tmp = raw;
    setRAWImage(tmp);
}

void myFrame::setRAWImage(ushort * RAWData)
{
    //imageData = new ushort[IMAGE_HEIGHT*IMAGE_WIDTH];
    //memcpy(imageData, RAWData, IMAGE_HEIGHT*IMAGE_WIDTH*2);

    qDebug() << "set raw image 1" << QTime::currentTime().toString("hh:mm:ss.zzz");
    quint8 * line = rxImage.scanLine(0);
    int stride =  rxImage.bytesPerLine();
    qDebug() << "set raw image 2" << QTime::currentTime().toString("hh:mm:ss.zzz");
    qint32 pixel = 0;
    for ( int y = 0; y < rxImage.height(); ++y, line += stride)
    {
        quint8 * pix = line;
        for ( int x = 0; x < rxImage.width(); ++x, pix += 1)
        {
            pixel  = RAWData[(y*IMAGE_WIDTH)+x]/256;
            if (pixel > 255)
            {
                pixel = 255;
            }
            if (pixel < 0)
            {
                pixel = 0;
            }
            pix[0] =  pixel;
        }
    }
    qDebug() << "set raw image 3" << QTime::currentTime().toString("hh:mm:ss.zzz");
    QPixmap pixmap;
    pixmap.convertFromImage(rxImage);

    scale = ((float)scrollArea->size().width()-2)/((float)rxImage.size().width());
    imageLabel->setPixmap(pixmap);

    //imageLabel->setPixmap((QPixmap::fromImage(imageLabel)));
    imageLabel->resize(rxImage.size()*scale);


    // Расчитываем гистограмму изображения
    qDebug() << "hist calculation 1" << QTime::currentTime().toString("hh:mm:ss.zzz");
    for (int i=0; i < (BITS + 1); i++) a[i] = 0;

    for ( int y = 0; y < rxImage.height(); y++)
    {
        for ( int x = 0; x < rxImage.width(); x++)
        {
            a[RAWData[(y*IMAGE_WIDTH)+x]]++;
        }
    }
    qDebug() << "hist calculation 2" << QTime::currentTime().toString("hh:mm:ss.zzz");
    emit histCalculated(a);
    emit finished();
}

void myFrame::onChangeHistogrammWidget(int x1, int x2)
{
    qDebug() << "ChangeHistogrammWidget 1" << QTime::currentTime().toString("hh:mm:ss.zzz");
    quint8 * line = rxImage.scanLine(0);
    int stride =  rxImage.bytesPerLine();
    qint32 pixel = 0;
    qDebug() << "ChangeHistogrammWidget 2" << QTime::currentTime().toString("hh:mm:ss.zzz");
    for ( int y = 0; y < rxImage.height(); ++y, line += stride)
    {
        quint8 * pix = line;
        for ( int x = 0; x < rxImage.width(); ++x, pix += 1)
        {
            pixel  = 255*(imageData[(y*IMAGE_WIDTH)+x]-x1)/((x2-x1));
            if (pixel > 255)
            {
                pixel = 255;
            }
            if (pixel < 0)
            {
                pixel = 0;
            }
            pix[0] =  pixel;
        }
    }
    qDebug() << "ChangeHistogrammWidget 3" << QTime::currentTime().toString("hh:mm:ss.zzz");

    QPixmap pixmap;
    pixmap.convertFromImage(rxImage);
    imageLabel->setPixmap(pixmap);
    qDebug() << "ChangeHistogrammWidget 4" << QTime::currentTime().toString("hh:mm:ss.zzz");
}


QImage myFrame::applyAutoContrast(int x1, int x2)
{
    qDebug() << "applyAutoContrast 1" << QTime::currentTime().toString("hh:mm:ss.zzz");
    quint8 * line = rxImage.scanLine(0);
    int stride =  rxImage.bytesPerLine();
    qint32 pixel = 0;
    for ( int y = 0; y < rxImage.height(); ++y, line += stride)
    {
        quint8 * pix = line;
        for ( int x = 0; x < rxImage.width(); ++x, pix += 1)
        {
            pixel  = 255*(imageData[(y*IMAGE_WIDTH)+x]-x1)/((x2-x1));
            if (pixel > 255)
            {
                pixel = 255;
            }
            if (pixel < 0)
            {
                pixel = 0;
            }
            pix[0] =  pixel;
        }
    }
    qDebug() << "applyAutoContrast 1" << QTime::currentTime().toString("hh:mm:ss.zzz");

    return rxImage;
}


//myFrame::~myFrame()
//{
//    disconnect(imageLabel, SIGNAL(wheel(QWheelEvent *)), this, SLOT(labelWheel(QWheelEvent *)));
//    disconnect(imageLabel, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(labelMousePress(QMouseEvent*)));
//    disconnect(imageLabel, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(labelMouseRelease(QMouseEvent*)));
//    disconnect(imageLabel, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(labelMouseMove(QMouseEvent*)));
//    disconnect(this, SIGNAL(resize(QResizeEvent *)), this, SLOT(scrollAreaResize(QResizeEvent*)));

//    delete imageLabel , layout , scrollArea , imageData;
//}
