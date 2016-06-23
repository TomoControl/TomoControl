#include "plcm.h"

PLCM::PLCM(QObject *parent) : QObject(parent)
{

}

void PLCM::connection()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "plcm::Имя        : " << info.portName();
        qDebug() << "plcm::Описание : " << info.description();
        qDebug() << "plcm::Производитель: " << info.manufacturer();
    }
    serial_plcm.setBaudRate(QSerialPort::Baud115200);
    serial_plcm.setPortName("COM1");
    serial_plcm.setDataBits(QSerialPort::Data8);
    serial_plcm.setParity(QSerialPort::NoParity);
    serial_plcm.setStopBits(QSerialPort::OneStop);
    serial_plcm.setFlowControl(QSerialPort::NoFlowControl);
    serial_plcm.open(QSerialPort::ReadWrite);
}

void PLCM::writeData(QByteArray data)
{
    serial_plcm.write(data);
}

void PLCM::initialization(uchar motor_number, ushort min_freq, ushort max_freq, ushort acceleration)
{
    ushort one = 23;
    uchar two = 21;
    sp.clear();
    sp.insert(sp.length(), 1);
    sp.insert(sp.length(), motor_number);
    sp.insert(sp.length(), (char*)&min_freq, 2);
    sp.insert(sp.length(), (char*)&max_freq, 2);
    sp.insert(sp.length(), (char*)&acceleration, 2);
    sp.insert(sp.length(), (char*)&one, 2);
    sp.insert(sp.length(), two);
    writeData(sp);
}

void PLCM::start(uchar motor_number, uchar DIR)
{
    uchar one = 20;
    sp.clear();
    sp.insert(sp.length(), 2);
    sp.insert(sp.length(), motor_number);
    sp.insert(sp.length(), DIR);
    sp.insert(sp.length(), one);
    writeData(sp);
}

void PLCM::stop(uchar motor_number)
{
    uchar one = 20;
    sp.clear();
    sp.insert(sp.length(), 3);
    sp.insert(sp.length(), motor_number);
    sp.insert(sp.length(), one);
    writeData(sp);
}

void PLCM::move(uchar motor_number, uchar DIR, int step)
{
    ushort one = 23;
    ushort two = 22;
    uchar three = 21;
    sp.clear();
    sp.insert(sp.length(), 4);
    sp.insert(sp.length(), motor_number);
    sp.insert(sp.length(), DIR);
    sp.insert(sp.length(), (char*)&step, 4);
    sp.insert(sp.length(), (char*)&one,2);
    sp.insert(sp.length(), (char*)&two,2);
    sp.insert(sp.length(), three);
    writeData(sp);
}

void PLCM::off()
{
    sp.clear();
    sp.insert(sp.length(),5);
    uchar one = 20;
    sp.insert(sp.length(),one);
    writeData(sp);
}

void PLCM::readData()
{
    QByteArray ARead;
    uint size = serial_plcm.bytesAvailable();
    uint nBlock = size / 9;

    QByteArray reading = ARead;
    qDebug() << reading << "plcm:: INPUT DATA";

    if (nBlock > 0)
    {
        ARead = serial_plcm.readAll();
        char tmp = ARead.at(1);
        switch (tmp)
        {
        case 0x06:
        {
            qDebug() << "mistake";
            break;
        }
        case 0x07:
        {
            qDebug() << "command accept";
            break;
        }
        case 0x0A:
        {
            qDebug() << "stop";
            break;
        }
        default:
            break;
        }
    }
}
