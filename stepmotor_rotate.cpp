 #include "stepmotor_rotate.h"


stepmotor_rotate::stepmotor_rotate(QObject *parent) : QObject(parent)
{
    MoveStatus = 0;
    current_position = 0;
    calb_axes = {0};
}

void stepmotor_rotate::initialization(QHostAddress Source, QHostAddress Destination, uint SourcePort, uint DestinationPort, uchar ControlNum)
{
    source = Source; // IP-адресс отправителя
    sourcePort = SourcePort; // порт отправителя
    destination = Destination; // IP-адресс получателя
    destinationPort = DestinationPort; // порт получателя
    if(ControlNum == 1) ControllerNumber = 1;
    if(ControlNum == 2) ControllerNumber = 2;
    client = new QUdpSocket;
    client->bind(source,sourcePort);
    connect(client, SIGNAL(readyRead()), this, SLOT(onClientReadyRead()));
    Counter = 0x01; // счетчик для номеров пакетов (с 0 не работает - особенность контроллера PLCM)
    CounterForWrite = 0; // счетчик числа пришедших статусных пакетов
    status_of_moving = 0; // идентификатор траекторных или обычных перемещений
    status_calb = 0; // идентификатор общего процесса калибровки
    numOfAxes = 3;
    direction = 0;
    stats = 0;
    for (uint i = 0; i < numOfAxes; i++) // переменные для калибровки
    {
        calibr[i] = 0;
        Position[i] = 0;
        CountStepOfAxe[i] = 0;
    }

    make_reset(); //сброс
    List.clear(); // очистка списка


    // read from settings.ini
    /* if (ControllerNumber == 1)
    {
        QSettings *setting_2 = new QSettings ( "settings.ini" , QSettings::IniFormat );
        position = {0};
        position.Position_1 = setting_2->value("CurrentPosition/positionX" , 0).toInt();
        position.Position_2 = setting_2->value("CurrentPosition/positionY" , 0).toInt();
        position.Position_3 = setting_2->value("CurrentPosition/positionZ" , 0).toInt();
        CountStepOfAxe[0] = setting_2->value("LengthOfAxe/lenghtZ", 0).toInt();
        CountStepOfAxe[1] = setting_2->value("LengthOfAxe/lenghtY", 0).toInt();
        CountStepOfAxe[2] = setting_2->value("LengthOfAxe/lenghtX", 0).toInt();
        Lim_L[0] = setting_2->value("LeftLimit/Lim_Z" , 0).toInt();
        Lim_L[1] = setting_2->value("LeftLimit/Lim_Y" , 0).toInt();
        Lim_L[2] = setting_2->value("LeftLimit/Lim_X" , 0).toInt();
        Lim_R[0] = setting_2->value("RightLimit/Lim_Z" , 0).toInt();
        Lim_R[1] = setting_2->value("RightLimit/Lim_Y" , 0).toInt();
        Lim_R[2] = setting_2->value("RightLimit/Lim_X" , 0).toInt();
    }
    if (ControllerNumber == 2)
    {
        QSettings *setting_2 = new QSettings ( "settings2.ini" , QSettings::IniFormat );
        position = {0};
        position.Position_1 = setting_2->value("CurrentPosition/positionX" , 0).toInt();
        position.Position_2 = setting_2->value("CurrentPosition/positionY" , 0).toInt();
        position.Position_3 = setting_2->value("CurrentPosition/positionZ" , 0).toInt();
        CountStepOfAxe[0] = setting_2->value("LengthOfAxe/lenghtZ", 0).toInt();
        CountStepOfAxe[1] = setting_2->value("LengthOfAxe/lenghtY", 0).toInt();
        CountStepOfAxe[2] = setting_2->value("LengthOfAxe/lenghtX", 0).toInt();
        Lim_L[0] = setting_2->value("LeftLimit/Lim_Z" , 0).toInt();
        Lim_L[1] = setting_2->value("LeftLimit/Lim_Y" , 0).toInt();
        Lim_L[2] = setting_2->value("LeftLimit/Lim_X" , 0).toInt();
        Lim_R[0] = setting_2->value("RightLimit/Lim_Z" , 0).toInt();
        Lim_R[1] = setting_2->value("RightLimit/Lim_Y" , 0).toInt();
        Lim_R[2] = setting_2->value("RightLimit/Lim_X" , 0).toInt();
    }*/

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
     if (calb_axes.a1 == 1)
     {
         if(MoveStatus && (Position[1] == need_position))
         {
            qDebug() << "calib" << position << need_position;
            MoveStatus = 0;
            if(job == 0) emit continue_scan();
         }
     }
     if (calb_axes.a2 == 1)
     {
         if(MoveStatus && (Position[2] == need_position))
         {
            qDebug() << "compare" << position << need_position;
            MoveStatus = 0;
            emit continue_scan();
         }
     }
     if (calb_axes.a4 == 1)
     {
         if(MoveStatus && (Position[0] == need_position))
         {
            qDebug() << "calib" << position << need_position;
            MoveStatus = 0;
            if(job == 0) emit continue_scan();
         }
     }

 }

 // setter на выбор количества калибруемых осей
 void stepmotor_rotate::setCalibrAxe( uchar Axe_1 , uchar Axe_2 , uchar Axe_3 )
 {
     AxeOfCalibr_1 = Axe_1;
     AxeOfCalibr_2 = Axe_2;
     AxeOfCalibr_3 = Axe_3;
     numOfAxes = AxeOfCalibr_1 + AxeOfCalibr_2 + AxeOfCalibr_3;
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

    // определение текущих координат
    position_1 = status->Position_0;
    position_2 = status->Position_1;
    position_3 = status->Position_2;
    position_4 = status->Position_3;
    position_5 = status->Position_4;
    position_6 = status->Position_5;
    Position[0] = status->Position_3;
    Position[1] = status->Position_0;
    Position[2] = status->Position_1;

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
    identif_conc();
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

// getter на индикацию пределов перемещения
limits_position stepmotor_rotate::get_limits()
{
    limits_position lim;
    lim.Lim_Axe_0_L = Lim_L[0];
    lim.Lim_Axe_0_R = Lim_R[0];
    lim.Lim_Axe_1_L = Lim_L[1];
    lim.Lim_Axe_1_R = Lim_R[1];
    lim.Lim_Axe_2_L = Lim_L[2];
    lim.Lim_Axe_2_R = Lim_R[2];
    lim.CountStepOfAxe_X = CountStepOfAxe[2];
    lim.CountStepOfAxe_Y = CountStepOfAxe[1];
    lim.CountStepOfAxe_Z = CountStepOfAxe[0];
    return lim;
}


// getter на текущие координаты
show_current_position stepmotor_rotate::get_current_position()
{
    show_current_position pos;
    pos.Position_1 = Position[2];
    pos.Position_2 = Position[1];
    pos.Position_3 = Position[0];
    return pos;
}


// функция идентификации концевиков и выбора режима работы системы
void stepmotor_rotate::identif_conc()
{
    if (status_calb == 0)
    {
        see_limits();
    }
    else
    {
        calibrate(); // работа с концевыми датчиками в режиме калибровки
    }
}

// работа концевиков в режиме ручного перемещения - остановка движения по конкретной оси
// работа концевиков в режиме траектроных данных - перевод драйвера в режим аварии
void stepmotor_rotate::see_limits()
{
    for (uint i = 0; i < numOfAxes; i++)
    {
        if (concev[i] == 0)
        {
            switch (status_of_moving)
            {
            case 0:
                Axes_Mask Axes;
                Axes = reset_axes_mask();
                if (i == 0)Axes.a4 = 1;
                if (i == 1)Axes.a1 = 1;
                if (i == 2)Axes.a2 = 1;
                stop_movement(Axes);
                break;
            case 1:
                go_emergency();
                break;
            default:
                break;
            }
        }
    }
}


// калибровка системы
void stepmotor_rotate::calibrate()
{
        if(status_calb == 0)
        {
            int Max_frequency;
            Axes_Mask Axes;
            Axes = reset_axes_mask();
            Axes.a4 = 1;
            Axes.a1 = 1;
            Axes.a2 = 1;
            Max_frequency = 5000;
            manual_movement ( Axes, Max_frequency );
            status_calb = 1;
            for (uint i = 0; i < numOfAxes; i++)
            {
                calibr[i] = 1;
                max_frequency[i] = 5000;
            }
        }
        else
        {
            for (uint i = 0; i < numOfAxes; i++)
            {
                if(concev[i] == 0 && calibr[i] == 1) // концевик итый сработал и статус калибровки оси 1
                {
                    calibr[i] = 2;
                    Axes_Mask Axes;
                    Axes.a4 = 1;
                    Axes.a1 = 1;
                    Axes.a2 = 1;
                    stop_movement(Axes);
                }

                if(concev[i] == 0 && calibr[i] == 2 && job == 0) // концевик итый сработал и статус калибровки оси 2 и не выполняется траекторных данных
                {
                    calb_traj_data(i, max_frequency[i]);
                }

                if(concev[i] == 1 && calibr[i] == 2 && job == 0)
                {
                    Axes_Mask Axes;
                    if (calibr[0] == 1 || calibr[0] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a4 = 1;
                        manual_movement(Axes, max_frequency[0]);
                    }
                    if (calibr[1] == 1 || calibr[1] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a1 = 1;
                        manual_movement(Axes, max_frequency[1]);
                    }
                    if (calibr[2] == 1 || calibr[2] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a2 = 1;
                        manual_movement(Axes, max_frequency[2]);
                    }
                    Axes = reset_axes_mask();
                    if (i == 0)Axes.a4 = 1;
                    if (i == 1)Axes.a1 = 1;
                    if (i == 2)Axes.a2 = 1;
                    Lim_R[i] = Position[i];
                    max_frequency[i] = ~max_frequency[i];
                    manual_movement(Axes, max_frequency[i]);
                    calibr[i] = 3;
                }

                if(concev[i] == 0 && calibr[i] == 3)
                {
                    Axes_Mask Axes;
                    Axes = reset_axes_mask();

                    Axes.a4 = 1;
                    Axes.a1 = 1;
                    Axes.a2 = 1;
                    stop_movement(Axes);
                    calibr[i] = 4;
                }

                if(concev[i] == 0 && calibr[i] == 4 && job == 0)
                {
                    calb_traj_data(i, max_frequency[i]);
                }

                if(concev[i] == 1 && calibr[i] == 4 && job == 0)
                {
                    Axes_Mask Axes;
                    if (calibr[0] == 1 || calibr[0] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a4 = 1;
                        manual_movement(Axes, max_frequency[0]);
                    }
                    if (calibr[1] == 1 || calibr[1] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a1 = 1;
                        manual_movement(Axes, max_frequency[1]);
                    }
                    if (calibr[2] == 1 || calibr[2] == 3)
                    {
                        Axes = reset_axes_mask();
                        Axes.a2 = 1;
                        manual_movement(Axes, max_frequency[2]);
                    }
                    Lim_L[i] = Position[i];
                    calibr[i] = 0;
                }

                uchar endOfCalibration = 0;
                for (uint i = 0; i < numOfAxes; i++)
                {
                    endOfCalibration += calibr[i];
                }
                if (endOfCalibration == 0)
                {
                    status_calb = 0;
                    for (uint i = 0; i < numOfAxes; i++)
                    {
                        CountStepOfAxe[i] = Lim_R[i] + abs(Lim_L[i]);
                    }
                }
                if( endOfCalibration == 0 && status_calb == 0 && stats == 0 && numOfAxes == 2)
                {
                    CountOfStep Step;
                    Step.Axe_X = 0;
                    Step.Axe_Y = CountStepOfAxe[1]/2;
                    Step.Axe_Z = CountStepOfAxe[0] - 10000;;
                    //go_to(Step);
                    qDebug() << "numOfAxes == 2";
                    stats = 1;
                }

                if( endOfCalibration == 0 && status_calb == 0 && stats == 0 && numOfAxes == 3)
                {
                    CountOfStep Step;
                    Step.Axe_X = CountStepOfAxe[2] - 5000;
                    Step.Axe_Y = CountStepOfAxe[1]/2;
                    Step.Axe_Z = 10000;
                    //go_to(Step);
                    qDebug() << "numOfAxes == 3";
                    stats = 1;
                }
            }
        }
   }

// калибровка - пакеты
void stepmotor_rotate::calb_traj_data(uint i, int max_freq)
{
    QByteArray temp;
    Trajectory traject;
    traject.Identifier_1 = 0;
    traject.Identifier_2 = 0;
    traject.Identifier_3 = 0;
    traject.N = 1;
    if(i == 1)traject.Traject_for_axe_0 =  5*1048600;else traject.Traject_for_axe_0 = 0; // условие движения по оси
    if(i == 2)traject.Traject_for_axe_1 =  5*1048600;else traject.Traject_for_axe_1 = 0; // условие движения по оси
    traject.Traject_for_axe_2 = 0;                                                        // 0 - ось Z, 1 - ось Y, 2 - ось X
    if(i == 0)traject.Traject_for_axe_3 =  5*1048600;else traject.Traject_for_axe_3 = 0; // условие движения по оси
    if(max_freq > 0) // выбор направления движения
    {
        traject.Traject_for_axe_1 = ~traject.Traject_for_axe_1;
        traject.Traject_for_axe_3 = ~traject.Traject_for_axe_3;
        traject.Traject_for_axe_0 = ~traject.Traject_for_axe_0;
    }
    traject.Traject_for_axe_4 = 0;
    traject.Traject_for_axe_5 = 0;
    traject.Ncmd = (0x07);
    traject.Seq_Number = Counter;
    traject.size = sizeof(traject);
    traject.crc = crc16((uchar*)&traject,traject.size-2);
    temp.insert(0,(char*)&traject, traject.size);
    List.push_back(temp); // запись в буфер
    Counter++;
}

// расчет и перемещение на заданное число шагов с помощью траекторных данных
void stepmotor_rotate::go_to(int step,Axes_Mask axes)
{
    qDebug() << "stepmotor_rotate::перемещение";
    uint step_1000, step_100, step_10, step_1;
    QByteArray body;
    qDebug() << "step_left2" << step;
    calb_axes = axes;
    direction = 0;
    if (step < 0)
    {
        direction = 1;
        step *= -1;
        qDebug() << "step_left3" << step;
    }

    step_1000 = step / 1000;
    step = step % 1000;
    step_100 = step / 100;
    step = step % 100;
    step_10 = step / 10;
    step_1 = step % 10;
    qDebug() << "step" << step_1000 << step_100 << step_10 << step_1;

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

//ljgbcfnm nhftrnjhrb
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

        // подписать оси

        // Траектория оси 1: - y поперечное

        if (calb_axes.a1 == 1)intT = packetScale * 1048600;
        else intT =  0;
        if(direction == 1) intT = ~intT;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 2:  - z вращение
        if (calb_axes.a2 == 1)intT = packetScale * 1048600;
        else intT =  0;
        if(direction == 1) intT = ~intT;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 3:
        intT = 0;
        pIntT = &intT;
        sp.insert(sp.length() , (char *)pIntT , 4);

        // Траектория оси 4:  - х продольное
        if (calb_axes.a4 == 1)intT = packetScale * 1048600;
        else intT =  0;
        if(direction == 1) intT = ~intT;
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
    emergency.crc = crc16((uchar*)&emergency,emergency.size-2);
    temp.insert(0,(char*)&emergency,emergency.size);
    List.push_front(temp);
    Counter++;
    // write to settings.ini
    if (ControllerNumber == 1)
    {
        QSettings *setting = new QSettings ( "settings.ini" , QSettings::IniFormat );
        setting->setValue("CurrentPosition/positionX" , position_2 );
        setting->setValue("CurrentPosition/positionY" , position_1 );
        setting->setValue("CurrentPosition/positionZ" , position_4 );
        setting->setValue("LengthOfAxe/lenghtX" , CountStepOfAxe[2]);
        setting->setValue("LengthOfAxe/lenghtY" , CountStepOfAxe[1]);
        setting->setValue("LengthOfAxe/lenghtZ" , CountStepOfAxe[0]);
        setting->setValue("LeftLimit/Lim_X" , Lim_L[2]);
        setting->setValue("LeftLimit/Lim_Y" , Lim_L[1]);
        setting->setValue("LeftLimit/Lim_Z" , Lim_L[0]);
        setting->setValue("RightLimit/Lim_X" , Lim_R[2]);
        setting->setValue("RightLimit/Lim_Y" , Lim_R[1]);
        setting->setValue("RightLimit/Lim_Z" , Lim_R[0]);
        setting->sync();
    }
    if (ControllerNumber == 2)
    {
        QSettings *setting = new QSettings ( "settings2.ini" , QSettings::IniFormat );
        setting->setValue("CurrentPosition/positionX" , position_2 );
        setting->setValue("CurrentPosition/positionY" , position_1 );
        setting->setValue("CurrentPosition/positionZ" , position_4 );
        setting->setValue("LengthOfAxe/lenghtX" , CountStepOfAxe[2]);
        setting->setValue("LengthOfAxe/lenghtY" , CountStepOfAxe[1]);
        setting->setValue("LengthOfAxe/lenghtZ" , CountStepOfAxe[0]);
        setting->setValue("LeftLimit/Lim_X" , Lim_L[2]);
        setting->setValue("LeftLimit/Lim_Y" , Lim_L[1]);
        setting->setValue("LeftLimit/Lim_Z" , Lim_L[0]);
        setting->setValue("RightLimit/Lim_X" , Lim_R[2]);
        setting->setValue("RightLimit/Lim_Y" , Lim_R[1]);
        setting->setValue("RightLimit/Lim_Z" , Lim_R[0]);
        setting->sync();
    }
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
    MoveStatus = 1;
    need_position = size_of_step * count;
    Axes_Mask axes;
    axes = reset_axes_mask();
    axes.a2 = 1;
    qDebug() << "stepmotor_rotate :: emit go" << need_position << position;
    if (need_position != position) go_to(size_of_step, axes);
}

void stepmotor_rotate::go_to_for_calb(int step,Axes_Mask axes)
{
    MoveStatus = 1;
    need_position = 0;
    if (axes.a1 == 1) {need_position = Position[1] + step; current_position = Position[1];}
    if (axes.a4 == 1) {need_position = Position[0] + step; current_position = Position[0];}
    if (need_position != position) go_to(step,axes);
}

Axes_Mask stepmotor_rotate::reset_axes_mask()
{
    Axes_Mask axes;
    axes.a1 = 0;
    axes.a2 = 0;
    axes.a3 = 0;
    axes.a4 = 0;
    axes.a5 = 0;
    axes.a6 = 0;
    axes.a7 = 0;
    axes.a8 = 0;
    return axes;
}

void stepmotor_rotate::setRunning(bool running)
{
    if (m_running == running)
        return;

    m_running = running;
    emit runningChanged(running);
}

bool stepmotor_rotate::running() const
{
    return m_running;
}

stepmotor_rotate::~stepmotor_rotate()
{
    QObject::disconnect(PacketTimer, SIGNAL(timeout()), this, SLOT(myPacketTimer()));
    //delete PacketTimer;    // TODO: We need to resolve this problem
    emit finished();
}

