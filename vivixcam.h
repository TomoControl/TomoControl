#ifndef VIVIXCAM_H
#define VIVIXCAM_H

#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QFile>
#include <QDate>

#include "Windows.h"

#include "ViVIX_SDK.h"

#include "cam.h"
#include "definitions.h"


using namespace Vieworks::vivix;

class ViVIXCam : public cam
{
    Q_OBJECT
public:

    virtual ~ViVIXCam();
    void InitializationCam();
    void mySleep(uint msec);
    void Disconnect();

    CVivixSDK * m_SDK;
    detector::CDetector* pDetector;

    ushort * data;

    HANDLE m_hNotifyFrameGrabberNormalImageInStart;
    HANDLE m_hNotifyFrameGrabberNormalImageIn;
    HANDLE m_hNotifyFrameGrabberNormalImageInEnd;
    Vieworks::vivix::detector::CFXRDW* pFXRDW;
    Vieworks::vivix::detector::CFXRDN* pFXRDN;

    static ViVIXCam* getInstance();

    static void NotifyFrameGrabberImageInStart(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode);
    static void NotifyFrameGrabberImageIn(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode, Vieworks::vivix::IMAGE_INFO* image);
    static void NotifyFrameGrabberImageInEnd(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode);
    static void NotifyExposed(Vieworks::vivix::DETECTOR_ID id);
    static void NotifyReadyForExposure(Vieworks::vivix::DETECTOR_ID id, bool ready);

signals:
    void GetDataComplete(ushort *);

protected:
    ViVIXCam();

private:
    void emitSignal(ushort *);
    static ViVIXCam* m_instance;

public slots:

    void AcquireImage();
    void SetAccumulationTime(int time);

private slots:
    void WaitForExecution();
};

#endif // VIVIXCAM_H
