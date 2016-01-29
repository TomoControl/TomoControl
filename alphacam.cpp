#include "alphacam.h"
#include <QTime>



AlphaCam::AlphaCam(QObject *parent) : QObject(parent)
{
    libStatus = false;

    QLibrary lib("APLink.dll");
    if(lib.load())
        qDebug() << "AlphaCam::Библиотека загружена";
    else
        qDebug() << "AlphaCam::Библиотека не загружена";

    // переопределение функций из сторонней библиотеки
    myAPServer_Connect = (AP_Connect) QLibrary::resolve("APLink.dll", "APServer_Connect");
    myAPServer_XRayShot = (AP_XRayShot) QLibrary::resolve("APLink.dll","APServer_XRayShot");
    myAPServer_AcquireDarkImage = (AP_AcquireDarkImage) QLibrary::resolve("APLink.dll","APServer_AcquireDarkImage");

    myAPServer_Disconnect = (AP_Disconnect) QLibrary::resolve("APLink.dll","APServer_Disconnect");
    myAPServer_ExecutionStatus = (AP_ExecutionStatus) QLibrary::resolve("APLink.dll","APServer_ExecutionStatus");
    myAPServer_ReadImageFormat = (AP_ReadImageFormat) QLibrary::resolve("APLink.dll","APServer_ReadImageFormat");
    myAPServer_ReadImage = (AP_ReadImage) QLibrary::resolve("APLink.dll","APServer_ReadImage");
    myAPServer_SetAccumulationTime = (AP_SetAccumulationTime) QLibrary::resolve("APLink.dll","APServer_SetAccumulationTime");

    libStatus = true;

    if (libStatus)
    {
        qDebug() << "AlphaCam::Установление соединения";
        HRESULT res;
        res = myAPServer_Connect();
    }
    number_of_steps = 0;
    count_of_steps = 0;
    ImageCount = 0;
}

AlphaCam::~AlphaCam()
{

    qDebug() << "AlphaCam::Отключение соединения";
    HRESULT res;
    res = myAPServer_Disconnect();
}

// команда на получение снимка
void AlphaCam::AcquireImage()
{
    QString time = QTime::currentTime().toString("hh-mm-ss.zz");
    qDebug() << "AlphaCam::AcquireImage" << time;

        data = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
        timer = new QTimer;
        connect(timer, SIGNAL(timeout()), this, SLOT(WaitForExecution()));

        HRESULT res;
        res = myAPServer_AcquireDarkImage();
        timer->start(500);
}

void AlphaCam::SetAccumulationTime(int time)
{
    HRESULT res;
    res = myAPServer_SetAccumulationTime(time);
}

// ожидание получения снимка
void AlphaCam::WaitForExecution()
{
    HRESULT res;
    res = myAPServer_ExecutionStatus();
    qDebug() << "res" << res;
    if (res == 0)
    {
        qDebug() << "AlphaCam::Статус выполнения:" << res;
        res = myAPServer_ReadImage((uchar*)data);
        emit GetDataComplete(data);
        timer->stop();
        delete timer;
    }
}

// извлечение полученного с камеры изображения
ushort * AlphaCam::GetData()
{
//    HRESULT res;
//    res = myAPServer_ReadImage((uchar*)data);
//    if (count_of_steps != number_of_steps)
//    {
//        qDebug() << "Alphacam::next_image";
//        emit disignation_for_go(size_of_step , count_of_steps);
//    }
//    else
//    {
//        qDebug() << "Alphacam::stop_scan";
//        emit stop_scan();
//        size_of_step = 0;
//        count_of_steps = 0;
//        number_of_steps = 0;
//    }
//    return data;
}

// отключение соединения с камерой
void AlphaCam::Disconnect()
{
    qDebug() << "AlphaCam::Отключение соединения";
    myAPServer_Disconnect();
}

void AlphaCam::onXrayFound()
{
    qDebug() << "AlphaCam::onXrayFound";
    AcquireImage();
}

void AlphaCam::get_number_of_step(int number)
{
    number_of_steps = number;
    size_of_step = FULL_TURN / number;
    qDebug() << "AutoScan:: Размер одного шага" <<  size_of_step;
}

std::string AlphaCam::int2str(int x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

QString AlphaCam::RenameOfImages()
{
    ImageCount ++;
    QString FormatOfName;
    FormatOfName = QString("%1").arg(ImageCount);
    while (FormatOfName.length() <= Format_Name_Of_Image)
    {
        FormatOfName = '0' + FormatOfName;
    }
    FormatOfName = "/image_" + FormatOfName + ".raw";
    if (ImageCount == number_of_steps) ImageCount = 0;
    qDebug() << "ImageCount" << ImageCount;
    return FormatOfName;
}

string AlphaCam::RenameOfImagesTiff()
{
    ImageCount ++;
    string FormatOfName;
    FormatOfName = int2str(ImageCount);

    while (FormatOfName.length() <= Format_Name_Of_Image)
    {
        FormatOfName = "0" + FormatOfName;
    }
    FormatOfName = "image_" + FormatOfName + ".tif";
    if (ImageCount == number_of_steps) ImageCount = 0;
    qDebug() << "ImageCount" << ImageCount << FormatOfName.data();
    return FormatOfName;
}
