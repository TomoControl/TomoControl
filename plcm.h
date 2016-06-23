#ifndef PLCM_H
#define PLCM_H

#include <QObject>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class PLCM : public QObject
{
    Q_OBJECT
public:
    explicit PLCM(QObject *parent = 0);

    QSerialPort serial_plcm;
    QByteArray sp;

    void writeData(QByteArray);


signals:
    void readyRead();

public slots:
    void connection();
    void initialization(uchar motor_number, ushort min_freq, ushort max_freq, ushort acceleration);
    void start(uchar motor_number, uchar DIR);
    void stop(uchar motor_number);
    void move(uchar motor_number, uchar DIR, int step);
    void off();
    void readData();
};

#endif // PLCM_H
