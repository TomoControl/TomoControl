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



//#define BUFFER_COUNT ( 16 )
typedef std::list<PvBuffer *> BufferList;

#define IMAGE_WIDTH 2304
#define IMAGE_HEIGHT 2944

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

    PvDevice * lDevice;
    PvDeviceInfo* lDeviceInfo;
    PvBuffer* gPvBuffers;
    PvStream* lStream;
    uint32_t gBufferCount;
    int64_t lWidth, lHeight, height;

    BufferList lBufferList;

    int number_of_steps;
    ushort ImageCount;

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
