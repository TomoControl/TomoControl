#ifndef ALPHACAM_H
#define ALPHACAM_H


#include <QMainWindow>
#include <QObject>
#include <QLibrary>
#include <QDebug>
#include <string.h>
#include <QTimer>
#include <QThread>

#include <string>
#include <sstream>
using namespace std;

#include "definitions.h"
#include "structures.h"

class AlphaCam : public QObject
{
    Q_OBJECT
public:
    explicit AlphaCam(QObject *parent = 0);
    ~AlphaCam();
    void mySleep (uint msec);

    int size_of_step;
    int number_of_steps, count_of_steps;

    ushort * data;
    ushort ImageCount;
    ushort Counter;

    bool libStatus;
    QTimer * timer;

    int bitCount;
    int m_nCCD_Width, m_nCCD_Height;



    typedef HRESULT (*AP_Connect)(void);
    typedef HRESULT (*AP_XRayShot)(void);
    typedef HRESULT (*AP_AcquireDarkImage)(void);
    typedef HRESULT (*AP_Disconnect)(void);
    typedef HRESULT (*AP_ReadImageFormat)(int*, int*, int*);
    typedef HRESULT (*AP_ExecutionStatus)(void);
    typedef HRESULT (*AP_ReadImage)(uchar*);
    typedef HRESULT (*AP_SetAccumulationTime)(int);

    AP_Connect myAPServer_Connect;
    AP_XRayShot myAPServer_XRayShot;
    AP_AcquireDarkImage myAPServer_AcquireDarkImage;
    AP_Disconnect myAPServer_Disconnect;
    AP_ExecutionStatus myAPServer_ExecutionStatus;
    AP_ReadImageFormat myAPServer_ReadImageFormat;
    AP_ReadImage myAPServer_ReadImage;
    AP_SetAccumulationTime myAPServer_SetAccumulationTime;

signals:
    void ExecutionComplete();
    void GetDataComplete(ushort*);
    void compare_coordinates();
    void disignation_for_go(int , int);
    void stop_scan();
    void next_image();
    void move_on(int,Axes_Mask);
    void finish();



public slots:
    void AcquireImage();
    void SetAccumulationTime(int time);
    ushort * GetData();

    void Disconnect();
    void onXrayFound();
    void get_number_of_step(int number);
    QString RenameOfImagesTiff();
    QString RenameOfImages();

private slots:
    void WaitForExecution();

};

#endif // ALPHACAM_H
