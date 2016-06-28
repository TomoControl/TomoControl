#ifndef MLTCAM_H
#define MLTCAM_H

#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEventLoop>

#include <PvSampleUtils.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvDeviceU3V.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvStreamU3V.h>
#include <PvBuffer.h>

#include <PvDeviceFinderWnd.h>

#include <definitions.h>
#include "cam.h"


typedef std::list<PvBuffer *> BufferList;


class MLTCam : public cam
{
public:
    MLTCam();
    ~MLTCam();
    void InitializationCam();
    void mySleep(uint msec);
    void CreateStreamBuffers( PvDevice *aDevice, PvStream *aStream, BufferList *aBufferList );
    PvStream * OpenStream( const PvDeviceInfo *aDeviceInfo );
    void ConfigureStream( PvDevice *aDevice, PvStream *aStream );
    void Acquire( PvDevice *aDevice, PvStream *aStream );
    void FreeStreamBuffers( BufferList *aBufferList );


    ushort * data;
    QTimer * timer;

    PvDevice * lDevice;
    PvDeviceInfo* lDeviceInfo;
    PvBuffer* gPvBuffers;
    PvStream* lStream;
    uint32_t gBufferCount;
    int64_t lWidth, lHeight, height;

    BufferList lBufferList;

    int number_of_steps;

public slots:
    virtual void AcquireImage();
    virtual void SetAccumulationTime(int time);
    virtual void Disconnect();

private slots:
    virtual void WaitForExecution();
};

#endif // MLTCAM_H
