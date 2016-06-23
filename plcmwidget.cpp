#include "plcmwidget.h"
#include "ui_plcmwidget.h"

plcmwidget::plcmwidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::plcmwidget)
{
    ui->setupUi(this);
    motor = new PLCM;
}

void plcmwidget::on_connect_clicked()
{
    motor->connection();
    connect(motor,SIGNAL(readyRead()),motor,SLOT(readData()));
}

void plcmwidget::on_init_clicked()
{
    uchar motor_num = ui->motor_number->text().toInt();
    ushort min_freq = ui->min_freq->text().toUShort();
    ushort max_freq = ui->max_freq->text().toUShort();
    ushort acc = ui->acceler->text().toUShort();
    motor->initialization(motor_num, min_freq, max_freq, acc);
}

void plcmwidget::on_start_clicked()
{
    uchar motor_num = ui->motor_number->text().toInt();
    uchar dir = ui->dir->text().toInt();
    motor->start(motor_num, dir);
}

void plcmwidget::on_stop_clicked()
{
    uchar motor_num = ui->motor_number->text().toInt();
    motor->stop(motor_num);
}

void plcmwidget::on_move_clicked()
{
    uchar motor_num = ui->motor_number->text().toInt();
    //uchar dir = ui->dir->text().toInt();
    int count = ui->count_of_step->text().toInt();
    uchar tmp;
    if (ui->right->isChecked()) tmp = 1;
    if (ui->left->isChecked()) tmp = 0;
    motor->move(motor_num,tmp,count);
}

void plcmwidget::on_off_clicked()
{
    motor->off();
}
plcmwidget::~plcmwidget()
{
    delete ui;
}
