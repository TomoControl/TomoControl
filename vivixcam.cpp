#include "vivixcam.h".

ViVIXCam* ViVIXCam::m_instance = 0;

ViVIXCam::ViVIXCam()
{

    data = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];

    m_SDK = new CVivixSDK;

    m_hNotifyFrameGrabberNormalImageInStart = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hNotifyFrameGrabberNormalImageIn = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hNotifyFrameGrabberNormalImageInEnd = CreateEvent(NULL, FALSE, FALSE, NULL);

    RESULT nResult = RESULT_ERROR;

    nResult = m_SDK->Initialize();
    if (nResult == RESULT_SUCCESS)
    {
        qDebug() << "ViVIXCAM::Succeeded to initialize VIVIX SDK";
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to initialize VIVIX SDK" << nResult;
        return;
    }

}

ViVIXCam::~ViVIXCam()
{
}

ViVIXCam* ViVIXCam::getInstance()
{
    if (m_instance == 0)
        m_instance = new ViVIXCam;
    return m_instance;
}

void ViVIXCam::emitSignal(ushort * tData)
{
   memcpy(data, tData, IMAGE_WIDTH*IMAGE_HEIGHT*2);
   emit GetDataComplete(data);
}

void ViVIXCam::InitializationCam()
{
    RESULT nResult = RESULT_ERROR;
    pDetector = NULL;
    nResult = m_SDK->GetDetector(0, &pDetector);
    if (nResult == Vieworks::vivix::RESULT_SUCCESS && pDetector != NULL)
    {
        qDebug() << "ViVIXCAM::Succeeded to get detector";
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to get detector" << nResult;
        return;
    }

    const Vieworks::vivix::DETECTOR_INFO* pDetectorInfo = NULL;
    nResult = m_SDK->DetectorInfoGet(0, &pDetectorInfo);
    if (nResult == Vieworks::vivix::RESULT_SUCCESS && pDetectorInfo != NULL)
    {
        qDebug() << "ViVIXCAM::Succeeded to get detector information";
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to get detector information" << nResult;
        return;
    }

    nResult = pDetector->Open();
    if (nResult == Vieworks::vivix::RESULT_SUCCESS)
    {
        qDebug() << "ViVIXCAM::Succeeded to open detector";
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to open detector" << nResult;
        return;
    }

    pDetector->SetNotifyFrameGrabberImageInStart(NotifyFrameGrabberImageInStart);
    pDetector->SetNotifyFrameGrabberImageIn(NotifyFrameGrabberImageIn);
    pDetector->SetNotifyFrameGrabberImageInEnd(NotifyFrameGrabberImageInEnd);

    switch (pDetectorInfo->DetectorType)
    {
    case Vieworks::vivix::DETECTOR_TYPE::FXRD1417_W:
    {
        pFXRDW = (Vieworks::vivix::detector::CFXRDW*)pDetector;
        pFXRDW->SetNotifyExposed(NotifyExposed);
        pFXRDW->SetNotifyReadyForExposure(NotifyReadyForExposure);
    }
    break;

    case Vieworks::vivix::DETECTOR_TYPE::FXRD1012_N:
    case Vieworks::vivix::DETECTOR_TYPE::FXRD1717_N:
    {
        pFXRDN = (Vieworks::vivix::detector::CFXRDN*)pDetector;
        pFXRDN->SetNotifyExposed(NotifyExposed);
        pFXRDN->SetNotifyReadyForExposure(NotifyReadyForExposure);
    }
    break;
    }

    Vieworks::vivix::CALIBRATION_READY_STATE CalibrationReadyState;
    nResult = pDetector->CalibrationReadyStateGet(&CalibrationReadyState);
    if (nResult == Vieworks::vivix::RESULT_SUCCESS)
    {
        if (CalibrationReadyState.OffsetIsReady == false)
        {
            nResult = pDetector->CalibrationLoad(Vieworks::vivix::CALIBRATION_FILE_OFFSET, pDetectorInfo->OffsetPath);
            if (nResult == Vieworks::vivix::RESULT_SUCCESS)
            {
                qDebug() << "ViVIXCAM::Succeeded to load offset calibration data file";
            }
            else
            {
                qDebug() << "ViVIXCAM::Failed to load offset calibration data file" << nResult;
                return;
            }
        }

        if (CalibrationReadyState.DefectIsReady == false)
        {
            nResult = pDetector->CalibrationLoad(Vieworks::vivix::CALIBRATION_FILE_DEFECT, pDetectorInfo->DefectPath);
            if (nResult == Vieworks::vivix::RESULT_SUCCESS)
            {
                qDebug() << "ViVIXCAM::Succeeded to load defect calibration data file";
            }
            else
            {
                qDebug() << "ViVIXCAM::Failed to load defect calibration data file" << nResult;
                return;
            }
        }

        if (CalibrationReadyState.GainIsReady == false)
        {
            nResult = pDetector->CalibrationLoad(Vieworks::vivix::CALIBRATION_FILE_GAIN, pDetectorInfo->GainPath);
            if (nResult == Vieworks::vivix::RESULT_SUCCESS)
            {
                qDebug() << "ViVIXCAM::Succeeded to load gain calibration data file";
            }
            else
            {
                qDebug() << "ViVIXCAM::Failed to load gain calibration data file" << nResult;
                return;
            }
        }
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to get calibration ready state" << nResult;
        return;
    }

    HANDLE hNotifyFrameGrabberNormalImage[3];

    hNotifyFrameGrabberNormalImage[0] = m_hNotifyFrameGrabberNormalImageInStart;
    hNotifyFrameGrabberNormalImage[1] = m_hNotifyFrameGrabberNormalImageIn;
    hNotifyFrameGrabberNormalImage[2] = m_hNotifyFrameGrabberNormalImageInEnd;

}

void ViVIXCam::Disconnect()
{
    RESULT nResult = RESULT_ERROR;
    nResult = pDetector->Close();
    if (nResult == Vieworks::vivix::RESULT_SUCCESS)
    {
        qDebug() << "ViVIXCAM::Succeeded to close detector";
    }
    else
    {
        qDebug() << "ViVIXCAM::Failed to close detector";
    return;
    }
    m_SDK->Destruct();
}

void ViVIXCam::SetAccumulationTime(int time)
{
    RESULT nResult = RESULT_ERROR;
    nResult = pDetector->ExposureTimeSet(time);
    if (nResult == Vieworks::vivix::RESULT_SUCCESS)
    {
        qDebug() << "ViVIXCAM::Accumulation time set to " << time;
    }
    else
    {
        qDebug() << "ViVIXCAM::Accumulation time set failed" << nResult;
        return;
    }
}

void ViVIXCam::AcquireImage()
{
    RESULT nResult = RESULT_ERROR;
    nResult = pDetector->SWTrigger();
    if (nResult == Vieworks::vivix::RESULT_SUCCESS)
    {
        qDebug() << "ViVIXCAM::SWTirgger";
    }
    else
    {
        qDebug() << "ViVIXCAM::SWTrigger is failed" << nResult;
        return;
    }
}


void ViVIXCam::WaitForExecution()
{
}


void ViVIXCam::NotifyFrameGrabberImageInStart(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode)
{
}

void ViVIXCam::NotifyFrameGrabberImageIn(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode, Vieworks::vivix::IMAGE_INFO * image)
{
    if (image != NULL)
    {
        qDebug() << "ViVIXCAM::Image Information";
        qDebug() << "ViVIXCAM::Width :" << image->Width;
        qDebug() << "ViVIXCAM::Height :" << image->Height;
        qDebug() << "ViVIXCAM::Using bit :" << image->UsingBit;
        if (id == 0 && imgMode == Vieworks::vivix::IMAGE_MODE::IMAGE_MODE_NORMAL)
        {
            qDebug() << "OnGetData";
//            ushort * dData;
//            dData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
//            memcpy(dData, (uchar *) image->Image, IMAGE_WIDTH * IMAGE_HEIGHT * 2);

//            QString FileDirectory = QString("image/Image_%1_%2.raw").arg(QDate::currentDate().toString("yyyy-MM-dd"))
//                                                     .arg(QTime::currentTime().toString("hh-mm-ss"));
//            // сохранение изображений в raw-формате
//            QFile file(FileDirectory);
//            if (!file.open(QIODevice::WriteOnly))
//            {
//                qDebug() << "Cканирование:: Сохранение изображения:: Ошибка открытия файла";
//            }
//            file.write((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);
//            delete[] dData;


            getInstance()->emitSignal((ushort *) image->Image);
        }
    }
}

void ViVIXCam::NotifyFrameGrabberImageInEnd(Vieworks::vivix::DETECTOR_ID id, Vieworks::vivix::IMAGE_MODE imgMode)
{

}

void ViVIXCam::NotifyExposed(Vieworks::vivix::DETECTOR_ID id)
{
    qDebug() << "NotifyExposed(%d)\n" << id;
}

void ViVIXCam::NotifyReadyForExposure(Vieworks::vivix::DETECTOR_ID id, bool ready)
{
    qDebug() << "NotifyReadyForExposure"  << id << ready;
}

