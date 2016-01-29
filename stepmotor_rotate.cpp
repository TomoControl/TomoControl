 #include "stepmotor_rotate.h"


stepmotor_rotate::stepmotor_rotate(QObject *parent) : QObject(parent)
{
    MoveStatus = 0;
}

void stepmotor_rotate::initialization(QHostAddress Source, QHostAddress Destination, uint SourcePort, uint DestinationPort)
{
    destination = Destination; // IP-адресс получателя
    destinationPort = DestinationPort; // порт получателя
    source = Source; // IP-адресс отправителя
    sourcePort = SourcePort; // порт отправителя
    client = new QUdpSocket;
    client->bind(source,sourcePort);
    connect(client , SIGNAL(readyRead()) , this , SLOT(onClientReadyRead()));
    Counter = 0x01; // счетчик для номеров пакетов (с 0 не работает - особенность контроллера PLCM)
    CounterForWrite = 0; // счетчик числа пришедших статусных пакетов

    make_reset(); //сброс
    List.clear(); // очистка списка

    set_start_position(); //установка текущих координат
    control_limit();
    setting_in_out_ports(); // настройка портов ввода-вывода
    set_velocity_factor(); // установка умножителя скорости
    compensation_lufts(); //компенсация люфтов

    PacketTimer = new QTimer;
    QObject::connect(PacketTimer , SIGNAL(timeout()) , this , SLOT(myPacketTimer()));
    PacketTimer->start(25);
    lastPacket = 1;
}

 void stepmotor_rotate::myPacketTimer()
 {
     lastPacket = 1;
     if(MoveStatus && (position == need_position))
     {
        qDebug() << "compare" << position << need_position;
        MoveStatus = 0;
        emit continue_move();
     }
 }

void stepmotor_rotate::onClientReadyRead() // чтение пришедшего статусного пакета
{
    QByteArray datagram;
    datagram.resize(client->pendingDatagramSize());
    client->readDatagram(datagram.data() , datagram.size());
    Status *status =(Status*)datagram.data();

    LastPerformedPacket = status->Ncmd;
    QByteArray pk;

    if (List.size() != 0)
    {
        iter = List.begin();
        pk = *iter;
        if ((unsigned char)pk.at(3) == LastPerformedPacket)
        {
            List.pop_front();
        }
    }
    else
    {
        CounterForWrite++;
        if (CounterForWrite == 3)
        {
            send_empty_command();
            CounterForWrite = 0;
        }
    }

    if (List.size() != 0)
    {
        if (lastPacket)
        {
            iter = List.begin();
            pk = *iter;
            client->writeDatagram( pk , destination , destinationPort );
            lastPacket = 0;
        }
    }

    // статус драйвера
    job = status->Job;

    // определение текущих координат
    position = status->Position_1;

    // индикация ошибки
    error_number = status->ErrCode;

    // индикация состояия концевых датчиков
    concev[0] = status->concevik.bit7;
    concev[1] = status->concevik.bit5;
    concev[2] = status->concevik.bit4;
    //identif_conc();
}

// getter на индикацию ошибки
char stepmotor_rotate::get_error()
{
    uchar err = error_number;
    return err;
}

// getter на индикацию статуса драйвера
char stepmotor_rotate::get_job()
{
    uchar status_of_driver = job;
    return  status_of_driver;
}

// getter на текущие координаты
show_current_position stepmotor_rotate::get_current_position()
{
    show_current_position positiont;
    positiont.Position_1 = 0;
    positiont.Position_2 = position;
    positiont.Position_3 = 0;
    positiont.Position_4 = 0;
    positiont.Position_5 = 0;
    positiont.Position_6 = 0;
    return positiont;
}

// расчет и перемещение на заданное число шагов с помощью траекторных данных
void stepmotor_rotate::go_to(int step)
{
    qDebug() << "stepmotor_rotate::перемещение";
    uint step_1000, step_100, step_10, step_1;
    QByteArray body;

    step_1000 = step / 1000;
    step = step % 1000;
    step_100 = step / 100;
    step = step % 100;
    step_10 = step / 10;
    step_1 = step % 10;


    for (uint k = 0; k < step_1000*2; k++)
    {
        body = BuildPackage(25 , 20); // 1000 шагов
        List.push_back(body);
    }

    for (uint k = 0; k < step_100; k++)
    {
        body = BuildPackage(5 , 20); // 100 шагов
        List.push_back(body);
    }

    for (uint k = 0; k < step_10; k++)
    {
        body = BuildPackage(1 , 10); // 10 шагов
        List.push_back(body);
    }

    for (uint k = 0; k < step_1; k++)
    {
        body = BuildPackage(1 , 1); // 1 шаг
        List.push_back(body);
    }
}

QByteArray stepmotor_rotate::BuildPackage(uchar packetScale, uchar nPacket)
{
    QByteArray sp;
    int numberData;
    int intT;
    ushort shortT;
    uchar charT;
    int * pIntT;
    ushort * pShortT;
    uchar * pCharT;

    // Номер команды:
    charT = 0x07;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);

    // Порядковый номер команды:
    charT = Counter;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);

    // Количество элементов траекторных данных
    numberData = nPacket;
    charT = numberData;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);

    // Идентификатор (нулевой)
    charT = 0;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);
    charT = 0;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);
    charT = 0;
    pCharT = &charT;
    sp.insert(sp.length() , (char *)pCharT , 1);

    for(int i = 0; i < numberData; i++)
    {
        // Траектория оси 1:

        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 2:
        intT = packetScale * 1048600;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 3:
        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 4:
        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 5:
        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 6:
        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);
    }
        // Длинна пакета:
        shortT = sp.length()+4;
        pShortT = &shortT;
        sp.insert(0, (char *)pShortT, 2);

        // Рассчет CRC16:
        ushort crc = crc16((uchar*)sp.data() , sp.length());
        pShortT = &crc;
        sp.insert(sp.length() , (char *)pShortT , 2);
        Counter++;
        return sp;
}


// отправка пустой команды для поддержания связи
void stepmotor_rotate::send_empty_command()
{
    QByteArray temp;
    Control_OutPorts control_outports;
    control_outports.Port_1 = 1;
    control_outports.Port_2 = 0;
    control_outports.Port_3 = 0;
    control_outports.PWM = 0;
    control_outports.Seq_Number = Counter;
    control_outports.Ncmd = 0x00;
    control_outports.size = sizeof(control_outports);
    control_outports.crc = crc16((uchar*)&control_outports , control_outports.size-2);
    temp.insert(0 ,(char*)&control_outports , control_outports.size);
    List.push_back(temp);
    Counter++;
}

// команда сброса, в ней содержится установка скорости движения осей системы
void stepmotor_rotate::make_reset()
{
    Reset reset;
    reset.Acceleration_0 = 160000;
    reset.Acceleration_1 = 160000;
    reset.Acceleration_2 = 160000;
    reset.Acceleration_3 = 160000;
    reset.Acceleration_4 = 160000;
    reset.Acceleration_5 = 160000;
    reset.Default_port_1 = 0;
    reset.Default_port_2 = 0;
    reset.Default_port_3 = 0;
    reset.Discretization_time = 5000;
    reset.Max_Velocity_0 = 2000;
    reset.Max_Velocity_1 = 2000;
    reset.Max_Velocity_2 = 2000;
    reset.Max_Velocity_3 = 2000;
    reset.Max_Velocity_4 = 2000;
    reset.Max_Velocity_5 = 2000;
    reset.MaxCorrAcc = 20;
    reset.Seq_Number = Counter;
    reset.Ncmd = (0x01);
    reset.size = sizeof(reset);
    reset.crc = crc16((uchar*)&reset , (reset.size-2));
    client->writeDatagram((char *)&reset , reset.size , destination,destinationPort);
    Counter++;
    qDebug() << "stepmotor_rotate::Сброс";
}


// перевод драйвера в режим аварии
void stepmotor_rotate::go_emergency()
{
    QByteArray temp;
    Emergency emergency;
    emergency.Seq_Number = LastPerformedPacket+1;
    emergency.Ncmd = (0x06);
    emergency.size = sizeof(emergency);
    emergency.crc = crc16((uchar*)&emergency , emergency.size-2);
    temp.insert(0 ,(char*)&emergency , emergency.size);
    List.push_front(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::Выключение";
}

// установка текущих координат
void stepmotor_rotate::set_start_position()
{
    QByteArray temp;
    Current_Position current_position;
    current_position.Position_0 = 0;
    current_position.Position_1 = 0;
    current_position.Position_2 = 0;
    current_position.Position_3 = 0;
    current_position.Position_4 = 0;
    current_position.Position_5 = 0;
    current_position.Seq_Number = Counter;
    current_position.Ncmd = (0x09);
    current_position.size = sizeof(current_position);
    current_position.crc = crc16((uchar*)&current_position , current_position.size-2);
    temp.insert(0 ,(char*)&current_position , current_position.size);
    List.push_back(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::Установка координат";
}

// включить\ выключить программное слежение за пределами
void stepmotor_rotate::control_limit()
{
    QByteArray temp;
    Control_Limits control_limit;
    control_limit.Enabled = 0; // 1 - включение программных пределов перемещения; 0 - выключение
    control_limit.Seq_Number = Counter;
    control_limit.Ncmd = (0x0b);
    control_limit.size = sizeof(control_limit);
    control_limit.crc = crc16((uchar*)&control_limit , control_limit.size-2);
    temp.insert(0 ,(char*)&control_limit , control_limit.size);
    List.push_back(temp);
    Counter++;
    if(control_limit.Enabled == 1)qDebug() << "stepmotor_rotate::Программный контроль за пределами перемещений включен";
    else qDebug() << "stepmotor_rotate::Программный контроль за пределами перемещений выключен";
}

// настройка портов ввода/вывода
void stepmotor_rotate::setting_in_out_ports()
{
    QByteArray temp;
    InOutPorts command;
    command.empt_1 = 11657;
    command.empt_2 = 65535;
    command.empt_3 = 65535;
    command.empt_4 = 65535;
    command.empt_5 = 65535;
    command.empt_6 = 65535;
    command.empt_7 = 65535;
    command.empt_8 = 35467;
    command.empt_9 = 35982;
    command.empt_10 = 65535;
    command.empt_11 = 65535;
    command.empt_12 = 65535;
    command.empt_13 = 65280;
    command.empt_14 = 65535;
    command.empt_15 = 65535;
    command.empt_16 = 65535;
    command.empt_17 = 65293;
    command.empt_18 = 258;
    command.empt_19 = 1023;
    command.empt_20 = 65535;
    command.empt_21 = 34182;
    command.empt_22 = 34815;
    command.empt_23 = 65535;
    command.Seq_Number = Counter;
    command.Ncmd = (0x0c);
    command.size = sizeof(command);
    command.crc = crc16((uchar*)&command , command.size-2);
    temp.insert(0,(char*)&command , command.size);
    List.push_back(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::Настройка портов ввода/вывода";
}


// установка множителя скорости
void stepmotor_rotate::set_velocity_factor()
{
    QByteArray temp;
    Velocity_factor velocity_factor;
    velocity_factor.Percent = 100;
    velocity_factor.Seq_Number = Counter;
    velocity_factor.Ncmd = (0x12);
    velocity_factor.size = sizeof(velocity_factor);
    velocity_factor.crc = crc16((uchar*)&velocity_factor , velocity_factor.size-2);
    temp.insert(0,(char*)&velocity_factor , velocity_factor.size);
    List.push_back(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::Установка значения умножителя скорости";
}

// компенсация люфтов
void stepmotor_rotate::compensation_lufts()
{
    QByteArray temp;
    Luft luft;
    luft.Backlash_0 = 0;
    luft.Backlash_1 = 0;
    luft.Backlash_2 = 0;
    luft.Backlash_3 = 0;
    luft.Backlash_4 = 0;
    luft.Backlash_5 = 0;
    luft.Seq_Number = Counter;
    luft.Ncmd = (0x11);
    luft.size = sizeof(luft);
    luft.crc = crc16((uchar*)&luft , luft.size-2);
    temp.insert(0,(char*)&luft , luft.size);
    List.push_back(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::Компенсация люфтов";
}

// функция ручного перемещения, Axes.N (битовая маска) = 1 - начать движение.
void stepmotor_rotate::manual_movement(Axes_Mask Axes,int Max_frequency)
{
    Jogging jogging;
    QByteArray temp;
    jogging.Max_frequency = Max_frequency;
    qDebug() << "max" << Max_frequency;
    jogging.Axes_mask = Axes;
    jogging.Seq_Number = Counter;
    jogging.Ncmd = (0x03);
    jogging.size = sizeof(jogging);
    jogging.crc = crc16((uchar*)&jogging,jogging.size-2);
    temp.insert(0,(char*)&jogging,jogging.size);
    List.push_back(temp);
    Counter++;
    qDebug() << "stepmotor_rotate::start move";
}

// остановка ручного перемещения
void stepmotor_rotate::stop_movement(Axes_Mask Axes)
{
    QByteArray temp;
    Stop stop;
    stop.Axes_mask = Axes;
    stop.Seq_Number = Counter;
    stop.Ncmd = (0x05);
    Counter++;
    stop.size = sizeof(stop);
    stop.crc = crc16((uchar*)&stop,stop.size-2);
    temp.insert(0,(char*)&stop,stop.size);
    List.push_back(temp);
    qDebug() << "stop move";
}

void stepmotor_rotate::calculate_go(int size_of_step, int count)
{
    qDebug() << "stepmotor_rotate :: emit go";
    MoveStatus = 1;
    need_position = size_of_step * count;
    if (need_position != position) go_to(size_of_step);
}

void stepmotor_rotate::identif_conc()
{
    for (uint i = 0; i < NUMBER_OF_AXES; i++)
    {
        if (concev[i] == 0)
        {
                Axes_Mask Axes;
                Axes = {0};
                if (i == 0)Axes.a4 = 1;
                if (i == 1)Axes.a1 = 1;
                if (i == 2)Axes.a2 = 1;
                stop_movement(Axes);
        }
    }
}

stepmotor_rotate::~stepmotor_rotate()
{
    QObject::disconnect(PacketTimer, SIGNAL(timeout()), this, SLOT(myPacketTimer()));
    delete PacketTimer;
}

