#ifndef stepmotor_rotate_H
#define stepmotor_rotate_H


#include <QObject>
#include <QSettings>

#include "QtNetwork/qudpsocket.h"
#include "crc16.h"
#include "structures.h"
#include "definitions.h"
#include <QTimer>


class stepmotor_rotate : public QObject
{
    Q_OBJECT
public:
    explicit stepmotor_rotate(QObject *parent = 0);
    ~stepmotor_rotate();

    show_current_position get_current_position();
    char get_error();
    char get_job();
    void initialization(QHostAddress Source, QHostAddress Destination, uint SourcePort, uint DestinationPort);
    void go_emergency();
    //void move_on_count_of_step(CountOfStep Count_of_step); // заданное число шагов

    void set_start_position(); // установка тек. координат
    void go_to(int step);


    list <QByteArray>::iterator iter;
    QUdpSocket * client;
    uint sourcePort,destinationPort;
    QHostAddress source,destination;
    uchar Counter, LastPerformedPacket;
    ushort CounterForWrite;
    QByteArray datagram;
    list <QByteArray> List;
    uchar error_number, job;
    uchar concev[3];
    int position;
    int need_position;
    ushort step_number;

    QTimer * PacketTimer;
    bool lastPacket, MoveStatus;

public slots:
    void onClientReadyRead();
    void myPacketTimer();
    void calculate_go(int,int);
    void manual_movement(Axes_Mask Axes, int Max_frequency);
    void stop_movement(Axes_Mask Axes);

signals:
    void continue_move();

private:

    void make_reset();
    void send_empty_command();
    void compensation_lufts();
    void control_limit();
    void enable_sensors();
    void set_velocity_factor();
    void setting_in_out_ports();
    QByteArray BuildPackage(uchar packetScale, uchar nPacket);
    void identif_conc();

};

#endif // stepmotor_rotate_H
