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
#include "cam.h"

class AlphaCam : public cam
{

public:
     AlphaCam();
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
    virtual void GetDataComplete(ushort*);



public slots:
    virtual void AcquireImage();
    virtual void SetAccumulationTime(int time);
    virtual void Disconnect();
    virtual QString RenameOfImagesTiff();
    virtual QString RenameOfImages();

private slots:
    void WaitForExecution();

};

#endif // ALPHACAM_H
