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
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

public:
    explicit stepmotor_rotate(QObject *parent = 0);
    ~stepmotor_rotate();

    show_current_position get_current_position();
    char get_error();
    char get_job();
    limits_position get_limits();

    void go_emergency();

    void set_start_position(); // установка тек. координат

    void setCalibrAxe(uchar Axe_1,uchar Axe_2,uchar Axe_3);
    void calb_traj_data(uint i, int max_freq);
    void calibrate();

    void see_limits();

    bool running() const; // thread

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
    int need_position, current_position;
    ushort step_number;
    uchar ControllerNumber;
    uchar direction;
    Axes_Mask calb_axes;

    QTimer * PacketTimer;
    bool lastPacket, MoveStatus;
    bool m_running; // thread

    uchar status_of_moving;
    int status_calb;
    uchar numOfAxes;
    uchar AxeOfCalibr_1,AxeOfCalibr_2,AxeOfCalibr_3;
    uchar stats;
    int calibr[5], max_frequency[3], Lim_R[3], Lim_L[3], Position[3], CountStepOfAxe[3];
    int position_1 , position_2 , position_3 , position_4 , position_5 , position_6;

public slots:
    void onClientReadyRead();
    void myPacketTimer();
    void calculate_go(int,int);
    void manual_movement(Axes_Mask Axes, int Max_frequency);
    void stop_movement(Axes_Mask Axes);
    void go_to(int step, Axes_Mask axes);
    void go_to_for_calb(int step, Axes_Mask axes);
    Axes_Mask reset_axes_mask();
    void setRunning(bool running); // thread
    void initialization(QHostAddress Source, QHostAddress Destination, uint SourcePort, uint DestinationPort, uchar ControlNum);


signals:
    void continue_move();
    void start_xray();
    void runningChanged(bool running); // thread
    void finished(); // thread

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
