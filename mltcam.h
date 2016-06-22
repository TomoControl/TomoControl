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

#include "definitions.h"


typedef std::list<PvBuffer *> BufferList;


class MLTCam : public QObject
{
    Q_OBJECT
public:
    explicit MLTCam(QObject *parent = 0);
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

    int number_of_steps, count_of_steps;
    ushort ImageCount;

    PvDevice * lDevice;
    PvDeviceInfo* lDeviceInfo;
    PvBuffer* gPvBuffers;
    PvStream* lStream;
    uint32_t gBufferCount;
    int64_t lWidth, lHeight;

    BufferList lBufferList;

signals:
    void GetDataComplete(ushort*);

public slots:
    void AcquireImage();
    void SetAccumulationTime(int time);

    QString RenameOfImagesTiff();
    QString RenameOfImages();
private slots:
    void WaitForExecution();
};

#endif // MLTCAM_H
