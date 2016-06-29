#include "mltcam.h"

MLTCam::MLTCam()
{
    data = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];

    PvDeviceFinderWnd lFinderWnd;
    if (! lFinderWnd.ShowModal().IsOK() )
    {
        return;
    }

    lDeviceInfo = (PvDeviceInfo*)lFinderWnd.GetSelected();

    PvResult lResult;

    if( lDeviceInfo != NULL )
    {
        qDebug() << "MLTCam::Connecting to"
                 << lDeviceInfo->GetManufacturerInfo().GetAscii();

        lDevice = PvDevice::CreateAndConnect(lDeviceInfo, &lResult );
        if ( lResult.IsOK() )
        {
            qDebug() <<  "MLTCam::Connected";
        }
    }
    else
    {
        qDebug() <<   "MLTCam::No device found";
    }


    lResult = lDevice->GetParameters()->GetIntegerValue("Height",height);
    if ( height !=  2944) InitializationCam();

    lStream = OpenStream( lDeviceInfo);
    qDebug() << "MLTCam::OpenStream";
    ConfigureStream( lDevice, lStream );
    qDebug() << "MLTCam::ConfigureStream";
    CreateStreamBuffers( lDevice, lStream, &lBufferList );
    qDebug() << "MLTCam::CreateStreamBuffers";


}

MLTCam::~MLTCam()
{
}

void MLTCam::InitializationCam()
{
    PvResult lResult;
    lResult = lDevice->GetParameters()->SetIntegerValue("AuthorizationCodeDFU", 0x77771111);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("TileSelector", "ReadAWriteA");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0x80000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0x80000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep(3000);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xB5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xB5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xC5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xC5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep(3000);
    lResult = lDevice->GetParameters()->SetIntegerValue("AuthorizationCodeDFU", 0x77771111);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("TileSelector", "ReadAWriteA");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xE5000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xE5000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep (100);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0x95000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0x95000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep (100);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep (100);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xB5000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xB5000001);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep (100);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xC5080000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xC5080000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep (100);
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("TileUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xA50000AA);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xB5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xB5000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0xC5200000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0xC5200000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AccessCodeDFU", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("RemoteUpdReg", 0x86000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->ExecuteCommand("RemoteUpdCom");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep(3000);
    lResult = lDevice->GetParameters()->SetIntegerValue("AuthorizationCodeDFU", 0x00000000);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("TileSelector", "ReadAWriteA");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("TileVoltageADCREF", 3400);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("InstalledTiles", "A");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("Binning", "Binning1x1");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("BinningMode", "Bsel1Bsel2");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    mySleep(1000);
    lResult = lDevice->GetParameters()->SetIntegerValue("Width", 2304);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("Height", 2944);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("TimeFix", 400);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("TimePulse", 60);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("Attenuation", "div1");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("FlashTime", 0);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("ReadDelay", 1);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("OffsetX", 0);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("OffsetY", 0);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("ReadingDirection", "FromCenter");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("GapMultiCapture", 0);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AutoDarkOffset", 0);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetEnumValue("TestImageSelector", "Off");
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("AcquisitionFrameCount", 1);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    lResult = lDevice->GetParameters()->SetIntegerValue("SyncConfiguration", 0x03700);
    if ( !lResult.IsOK()) qDebug() << "Error!";
    qDebug() << "MLTCam::Initialization continue";
}

void MLTCam::mySleep(uint msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}

void MLTCam::SetAccumulationTime(int time)
{
    PvResult aResult;
    aResult = lDevice->GetParameters()->SetEnumValue("AcquisitionMode", "SingleFrame");
    if ( !aResult.IsOK()) qDebug() << "Error!";
    aResult = lDevice->GetParameters()->SetEnumValue("SyncMode", "SoftwareTrigger");
    if ( !aResult.IsOK()) qDebug() << "Error!";
    aResult = lDevice->GetParameters()->SetIntegerValue("ExposureTimeInt", time);
    if ( !aResult.IsOK()) qDebug() << "Error!";
    qDebug() << "MLTCam::Exposure time set to " << time ;
}

void MLTCam::AcquireImage()
{
    Acquire( lDevice, lStream );
}

void MLTCam::CreateStreamBuffers( PvDevice *aDevice, PvStream *aStream, BufferList *aBufferList )
{
    // Reading payload size from device
    uint32_t lSize = aDevice->GetPayloadSize();

    // Use BUFFER_COUNT or the maximum number of buffers, whichever is smaller
    uint32_t lBufferCount = ( aStream->GetQueuedBufferMaximum() < BUFFER_COUNT ) ?
        aStream->GetQueuedBufferMaximum() :
        BUFFER_COUNT;

    // Allocate buffers
    for ( uint32_t i = 0; i < lBufferCount; i++ )
    {
        // Create new buffer object
        PvBuffer *lBuffer = new PvBuffer;

        // Have the new buffer object allocate payload memory
        lBuffer->Alloc( static_cast<uint32_t>( lSize ) );

        // Add to external list - used to eventually release the buffers
        aBufferList->push_back( lBuffer );
    }

    // Queue all buffers in the stream
    BufferList::iterator lIt = aBufferList->begin();
    while ( lIt != aBufferList->end() )
    {
        aStream->QueueBuffer( *lIt );
        lIt++;
    }
}

PvStream * MLTCam::OpenStream( const PvDeviceInfo *aDeviceInfo )
{
    PvStream *aStream;
    PvResult lResult;

    // Open stream to the GigE Vision or USB3 Vision device
    qDebug() << "Opening stream to device.";
    aStream = aStream->CreateAndOpen( aDeviceInfo->GetConnectionID(), &lResult );
    if ( aStream == NULL )
    {
        cout << "Unable to stream from " << aDeviceInfo->GetDisplayID().GetAscii() << "." << endl;
    }

    return aStream;
}

void MLTCam::ConfigureStream( PvDevice *aDevice, PvStream *aStream )
{
    // If this is a GigE Vision device, configure GigE Vision specific streaming parameters
    PvDeviceGEV* lDeviceGEV = dynamic_cast<PvDeviceGEV *>( aDevice );
    if ( lDeviceGEV != NULL )
    {
        PvStreamGEV *lStreamGEV = static_cast<PvStreamGEV *>( aStream );

        // Negotiate packet size
        lDeviceGEV->NegotiatePacketSize();

        // Configure device streaming destination
        lDeviceGEV->SetStreamDestination( lStreamGEV->GetLocalIPAddress(), lStreamGEV->GetLocalPort() );
    }
}

void MLTCam::Acquire( PvDevice *aDevice, PvStream *aStream )
{
    // Get device parameters need to control streaming
    PvGenParameterArray *lDeviceParams = aDevice->GetParameters();

    // Map the GenICam AcquisitionStart and AcquisitionStop commands
    PvGenCommand *lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
    PvGenCommand *lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );

    // Get stream parameters
    PvGenParameterArray *lStreamParams = aStream->GetParameters();

    // Map a few GenICam stream stats counters
    PvGenFloat *lFrameRate = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "AcquisitionRate" ) );
    PvGenFloat *lBandwidth = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "Bandwidth" ) );

    // Enable streaming and send the AcquisitionStart command
    //qDebug() << "Enabling streaming and sending AcquisitionStart command." << endl;
    aDevice->StreamEnable();
    lStart->Execute();

    char lDoodle[] = "|\\-|-/";
    int lDoodleIndex = 0;
    double lFrameRateVal = 0.0;
    double lBandwidthVal = 0.0;


    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), SLOT(WaitForExecution()));
    timer->start(100);

    // Abort all buffers from the stream and dequeue
//    cout << "Aborting buffers still in stream" << endl;
//    aStream->AbortQueuedBuffers();
//    while ( aStream->GetQueuedBufferCount() > 0 )
//    {
//        PvBuffer *lBuffer = NULL;
//        PvResult lOperationResult;

//        aStream->RetrieveBuffer( &lBuffer, &lOperationResult );
//    }
}

void MLTCam::WaitForExecution()
{
    qDebug() <<  "MLTCam::Wait for execution...";
    PvBuffer *lBuffer = NULL;
    PvResult lOperationResult;

    // Retrieve next buffer
    PvResult lResult = lStream->RetrieveBuffer( &lBuffer, &lOperationResult, 1000 );
    if ( lResult.IsOK() )
    {
        if ( lOperationResult.IsOK() )
        {
            qDebug() <<  "MLTCam::Get image complete";
            PvImage *lImage2 = lBuffer->GetImage();
            ushort * tdata = (ushort*)lImage2->GetDataPointer();

            memcpy(data, tdata, IMAGE_WIDTH*IMAGE_HEIGHT*2);

            timer->stop();
            delete timer;

            emit GetDataComplete(data);
            lDevice->StreamDisable();
        }

        // Re-queue the buffer in the stream object
        lStream->QueueBuffer( lBuffer );
    }

}


void MLTCam::FreeStreamBuffers( BufferList *aBufferList )
{
    // Go through the buffer list
    BufferList::iterator lIt = aBufferList->begin();
    while ( lIt != aBufferList->end() )
    {
        delete *lIt;
        lIt++;
    }

    // Clear the buffer list
    aBufferList->clear();
}


void MLTCam::Disconnect()
{
    qDebug() << "mltCam::Отключение соединения";
}
