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
//#pragma pack(push, 1)
struct InOutPorts
{
    ushort size;
    uchar Ncmd;
    uchar Seq_Number;
    uchar empt_1;
    uchar empt_2;
    uchar empt_3;
    uchar empt_4;
    uchar empt_5;
    uchar empt_6;
    uchar empt_7;
    uchar empt_8;
    uchar empt_9;
    uchar empt_10;
    uchar empt_11;
    uchar empt_12;
    uchar empt_13;
    uchar empt_14;
    uchar empt_15;
    uchar empt_16;
    uchar empt_17;
    uchar empt_18;
    uchar empt_19;
    uchar empt_20;
    uchar empt_21;
    uchar empt_22;
    uchar empt_23;
    uchar empt_24;
    uchar empt_25;
    uchar empt_26;
    uchar empt_27;
    uchar empt_28;
    uchar empt_29;
    uchar empt_30;
    uchar empt_31;
    uchar empt_32;
    uchar empt_33;
    uchar empt_34;
    uchar empt_35;
    uchar empt_36;
    uchar empt_37;
    uchar empt_38;
    uchar empt_39;
    uchar empt_40;
    uchar empt_41;
    uchar empt_42;
    uchar empt_43;
    uchar empt_44;
    uchar empt_45;
    uchar empt_46;
    uchar empt_47;
    uchar empt_48;
    ushort crc;
};
//#pragma pack(pop)

// структура для отображения текущей позиции
struct show_current_position
{
    int Position_1;
    int Position_2;
    int Position_3;
};


// текущие координаты для go_to
struct Cur_Pos
{
    int Position_X;
    int Position_Y;
    int Position_Z;
};

struct CountOfStep
{
    int Axe_X;
    int Axe_Y;
    int Axe_Z;
};

// отображение пределов перeмещения
struct limits_position
{
    int Lim_Axe_0_R;
    int Lim_Axe_0_L;
    int Lim_Axe_1_R;
    int Lim_Axe_1_L;
    int Lim_Axe_2_R;
    int Lim_Axe_2_L;
    int CountStepOfAxe_X;
    int CountStepOfAxe_Y;
    int CountStepOfAxe_Z;
};

#endif // STRUCTURES

