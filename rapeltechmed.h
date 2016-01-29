#ifndef RAPELTECHMED_H
#define RAPELTECHMED_H

#include <QMainWindow>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QLibrary>



class RAPEltechMED : public QObject
{
    Q_OBJECT
public:
    explicit RAPEltechMED(QObject *parent = 0);
    void writeData(uchar nCmd, uchar data1, uchar data2, uchar data3);
    void setU(uchar kV);
    void setI(ushort mkA);

    uint getU();
    uint getI();
    uchar getStatus();
    bool getXrayStatus();
    void initialization();

    typedef int (*RAP_SetU)(uint);
    typedef int (*RAP_SetI)(uint);
    typedef int (*RAP_GetU)(void);
    typedef int (*RAP_GetI)(void);
    typedef int (*RAP_GetInfo)(void);
    typedef int (*RAP_On_Off)(void);

    RAP_SetU libSetU;
    RAP_SetI libSetI;
    RAP_GetU libGetU;
    RAP_GetI libGetI;
    RAP_GetInfo libGetInfo;
    RAP_On_Off libOn_Off;

    QSerialPort *serial;
    QByteArray sp;
    uchar *rxData;
    QTimer *timer;

    uint currentI, currentU, needI, needU;

    uchar currentStatus;
    bool xrayStatus, xrayF;


signals:
    void connected();
    void error();
    void xrayOn();
    void xrayOff();
    void xrayFound();
    void changeU(uint);
    void changeI(uint);

public slots:
    void writeStatus();
    void readData();
    void updateCaption();
    void ClosePort();
    void on(ushort kV, ushort mkA);
    void off();
};

#endif // RAPELTECHMED_H
