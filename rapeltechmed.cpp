#include "rapeltechmed.h"

RAPEltechMED::RAPEltechMED(QObject *parent) :
    QObject(parent)
{
    // Выводим зарегистрированные в системе COM порты

    currentI = 0;
    currentU = 0;
    needI = 0;
    needU = 0;
    xrayStatus = 0;
    xrayF = 0;
}

void RAPEltechMED::initialization()
{
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        qDebug() << "RAP::Имя        : " << info.portName();
        qDebug() << "RAP::Описание : " << info.description();
        qDebug() << "RAP::Производитель: " << info.manufacturer();
    }
    serial = new QSerialPort(this);
    serial->setBaudRate(QSerialPort::Baud19200);
    serial->setPortName("COM4");

    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));

    if (serial->open(QIODevice::ReadWrite))
    {
        writeData(106, 255, 255, 255);
    }

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateCaption()));
}

void RAPEltechMED::writeData(uchar nCmd, uchar data1, uchar data2, uchar data3)
{
    // Очищаем буфер отправки
    sp.clear();
    // Номер команды
    sp.insert(sp.length(), nCmd);
    // Данные
    sp.insert(sp.length(), data1);
    sp.insert(sp.length(), data2);
    sp.insert(sp.length(), data3);

    // Контрольная сумма
    uchar crc = nCmd + data1 + data2 + data3;
    sp.insert(sp.length(), crc);

    // Отправляем данные РА
    if (nCmd == 90) qDebug() << "RAP::Команда для отправки: " << nCmd;
    serial->write(sp);
}

void RAPEltechMED::readData()
{
//    qDebug() << "RAP::read";
    uint value;
    uint size = serial->bytesAvailable();
    uint nBlock = size / 5;

    if (nBlock > 0)
    {
        for (uint i = 0; i < nBlock; i++)
        {
            QByteArray data = serial->read(5);

            rxData = (uchar*)data.data();

            uchar crc = 0;
            crc = rxData[0]+rxData[1]+rxData[2]+rxData[3];

            if (crc != rxData[4]) return;

            switch (rxData[0])
            {
                case 90: // Изменение состояния рентгена
                    // Проверяем предыдущее состояние
                    if (xrayStatus == 0)
                    {
                        xrayStatus = 1;
                        qDebug() << "RAP::Включили рентген";
                        emit xrayOn();
                    }
                    else
                    {
                        xrayStatus = 0;
                        xrayF = 0;
                        currentI = 0;
                        currentU = 0;
                        qDebug() << "RAP::Выключили рентген";
                        emit changeU(0);
                        emit changeI(0);
                        emit xrayOff();
                    }
                break;

                case 106: // Ответ на команду проверки связи
                    if (rxData[1] == 0)
                    {
                        // Если связь установлена, то устанавливаем режим работы
                        writeData(129, 1, 0, 0);
                        qDebug() << "RAP::COM4 подключен";
                        emit connected();
                    }
                    else
                    {
                        emit error();
                    }
                break;

                case 129: // Подтверждение установки режима работы
                    // Включаем таймер обновления статуса прибора
                    timer->start(500);
                break;

                case 136: // Подтверждение установки напряжения
                    uchar *pmkA;
                    pmkA = (uchar*)&needI;
                    writeData(137, pmkA[0], pmkA[1], 0);
                break;

                case 137: // Подтверждение установки тока
                    writeData(90, 0, 0, 0);
                break;

                case 139: // Данные о текущем статусе аппарата
                    currentStatus = rxData[1];
                    writeData(141, 0, 0, 0);
                break;

                case 141: // Данные о текущем значении напряжения на трубке
                    value = (((int)rxData[1] << 8) | rxData[2]) / 10;
                    if (value != currentU)
                    {
                        currentU = value;
                        emit changeU(currentU);
                    }
                    writeData(142, 0, 0, 0);
                break;

                case 142: // Данные о текущем значении тока трубки
                    value = ((int)rxData[1] << 8) | rxData[2];
                    if (value != currentI)
                    {
                        currentI = value;
                        emit changeI(currentI);
                    }

                    if ((currentI > (needI-2))&&
                        (currentI < (needI+2))&&
                        (currentU > (needU-2))&&
                        (currentU < (needU+2)))
                    {
                        if (xrayF == 0 && xrayStatus)
                        {
                            emit xrayFound();
                            xrayF = 1;
                        }

                    }
                break;
            }
        }
    }

}

void RAPEltechMED::setU(uchar kV)
{
    writeData(136, kV, 0, 0);
    needU = kV;
}

void RAPEltechMED::setI(ushort mkA)
{
    uchar *pmkA;
    pmkA = (uchar*)&mkA;
    writeData(137, pmkA[0], pmkA[1], 0);
    needI = mkA;
}

void RAPEltechMED::on(ushort kV, ushort mkA)
{
    if (!xrayStatus)
    {
        writeData(136, kV, 0, 0);
        needU = kV;
        needI = mkA;
        qDebug() << "RAP::Команда на включение рентгена " << needU << "кВ " << needI << "мкА";
    }
}

void RAPEltechMED::off()
{
    if (xrayStatus)
    {
        currentI = 0;
        currentU = 0;
        writeData(90, 0, 0, 0);
    }

}

uint RAPEltechMED::getU()
{
    return currentU;
}

uint RAPEltechMED::getI()
{
    return currentI;
}

uchar RAPEltechMED::getStatus()
{
    return currentStatus;
}

bool RAPEltechMED::getXrayStatus()
{
    return xrayStatus;
}

void RAPEltechMED::writeStatus()
{
    writeData(139, 0, 0, 0);
}

void RAPEltechMED::updateCaption()
{
    if (xrayStatus)
    {
        writeData(139, 0, 0, 0);
    }
}

void RAPEltechMED::ClosePort()
{
    serial->close();
}


