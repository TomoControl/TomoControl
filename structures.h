#ifndef STRUCTURES
#define STRUCTURES

using namespace std;

struct Axes_Mask
{
    uchar a1:1;
    uchar a2:1;
    uchar a3:1;
    uchar a4:1;
    uchar a5:1;
    uchar a6:1;
    uchar a7:1;
    uchar a8:1;
};

struct bits
{
    uchar bit1:1;
    uchar bit2:1;
    uchar bit3:1;
    uchar bit4:1;
    uchar bit5:1;
    uchar bit6:1;
    uchar bit7:1;
    uchar bit8:1;
};

// статусный пакет, приходящий от контроллера
struct Status
{
    ushort size;
    uchar Ncmd;
    uchar Job;
    uchar Flags;
    uchar ErrCmdNum;
    uchar ErrCode;
    uchar Referenced;
    uint AxisJobs;
    int Position_0;
    int Position_1;
    int Position_2;
    int Position_3;
    int Position_4;
    int Position_5;
    uint Trajectory_count;
    uint System_time;
    uint Firmware_version;
    uint Port_1;
    uchar Port_2_begin;
    bits concevik;
    ushort Port_2_end;
    uint Port_3;
    int THC_correction;
    ushort THC_voltage;
    ushort reserved;
    ushort crc;
};

// Пакет ручного перемещения
#pragma pack(push, 1)
struct Jogging
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    int Max_frequency;
    Axes_Mask Axes_mask;
    ushort crc;
};
#pragma pack(pop)

// Пакет на остановку
#pragma pack(push, 1)
struct Stop
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    Axes_Mask Axes_mask;
    ushort crc;
};
#pragma pack(pop)

// Пакет сброса
#pragma pack(push, 1)
struct Reset
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uint Discretization_time;
    uint Acceleration_0;
    uint Acceleration_1;
    uint Acceleration_2;
    uint Acceleration_3;
    uint Acceleration_4;
    uint Acceleration_5;
    uint Max_Velocity_0;
    uint Max_Velocity_1;
    uint Max_Velocity_2;
    uint Max_Velocity_3;
    uint Max_Velocity_4;
    uint Max_Velocity_5;
    uint Default_port_1;
    uint Default_port_2;
    uint Default_port_3;
    uchar MaxCorrAcc;
    ushort crc;
};
#pragma pack(pop)

// Пакет пустой команды
#pragma pack(push, 1)
struct Control_OutPorts
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uint Port_1;
    uint Port_2;
    uint Port_3;
    uchar PWM;
    ushort crc;
};
#pragma pack(pop)


//Пакет траекторных данных
#pragma pack(push, 1)
struct Trajectory
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uchar N;
    uchar Identifier_1;
    uchar Identifier_2;
    uchar Identifier_3;
    int Traject_for_axe_0;
    int Traject_for_axe_1;
    int Traject_for_axe_2;
    int Traject_for_axe_3;
    int Traject_for_axe_4;
    int Traject_for_axe_5;
    ushort crc;
};
#pragma pack(pop)

// Пакет, перводящий контроллер в режим аварии
#pragma pack(push, 1)
struct Emergency
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    ushort crc;
};
#pragma pack(pop)

// Пакет установки текущих координат
#pragma pack(push, 1)
struct Current_Position
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uint Position_0;
    uint Position_1;
    uint Position_2;
    uint Position_3;
    uint Position_4;
    uint Position_5;
    ushort crc;
};
#pragma pack(pop)

// Пакет включения ограничения перемещений
#pragma pack(push, 1)
struct Control_Limits
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uchar Enabled;
    ushort crc;
};
#pragma pack(pop)

//Компенсация люфтов
#pragma pack(push, 1)
struct Luft
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uint Backlash_0;
    uint Backlash_1;
    uint Backlash_2;
    uint Backlash_3;
    uint Backlash_4;
    uint Backlash_5;
    ushort crc;
};
#pragma pack(pop)

// Пакет умножителя скорости
#pragma pack(push, 1)
struct Velocity_factor
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uchar Percent;
    ushort crc;
};
#pragma pack(pop)


// порты ввода-вывода
#pragma pack(push, 1)
struct InOutPorts
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    ushort empt_1;
    ushort empt_2;
    ushort empt_3;
    ushort empt_4;
    ushort empt_5;
    ushort empt_6;
    ushort empt_7;
    ushort empt_8;
    ushort empt_9;
    ushort empt_10;
    ushort empt_11;
    ushort empt_12;
    ushort empt_13;
    ushort empt_14;
    ushort empt_15;
    ushort empt_16;
    ushort empt_17;
    ushort empt_18;
    ushort empt_19;
    ushort empt_20;
    ushort empt_21;
    ushort empt_22;
    ushort empt_23;
    ushort crc;
};
#pragma pack(pop)

// структура для отображения текущей позиции
struct show_current_position
{
    int Position_1;
    int Position_2;
    int Position_3;
    int Position_4;
    int Position_5;
    int Position_6;
};


// текущие координаты для go_to
struct Cur_Pos
{
    int Position_X;
    int Position_Y;
    int Position_Z;
};

#endif // STRUCTURES

