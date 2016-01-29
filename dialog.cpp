#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    emit close_dialog();
}

void Dialog::set_icons()
{
    //size of icons
    QSize size, pic_size;
    size.setHeight(50);
    size.setWidth(50);
    pic_size.setHeight(100);
    pic_size.setWidth(100);

    //load icons
    QPixmap source_icon, reciever_icon, object_icon;
    source_icon.load("://resource/source.png");
    reciever_icon.load("://resource/camera.png");
    object_icon.load("://resource/object.png");
    source_icon.scaled(pic_size);
    reciever_icon.scaled(pic_size);
    object_icon.scaled(pic_size);

    QPixmap forward, back, right, left, left_rotate, right_rotate;
    forward.load("://resource/Arrow Right.ico");
    back.load("://resource/Arrow Left.ico");
    right.load("://resource/Arrow Down.ico");
    left.load("://resource/Arrow Up.ico");
    left_rotate.load("://resource/rotate_arrow_left.ico");
    right_rotate.load("://resource/rotate_arrow_right.ico");
    QIcon forward_icon, back_icon, right_icon, left_icon, left_r_icon, right_r_icon;
    forward_icon = (QIcon)forward;
    back_icon = (QIcon)back;
    left_icon = (QIcon)left;
    right_icon = (QIcon)right;
    left_r_icon = (QIcon)left_rotate;
    right_r_icon = (QIcon)right_rotate;

    //set icons
    // source
    ui->source->setPixmap(source_icon);

    ui->source_back->setText("");
    ui->source_back->setIconSize(size);
    ui->source_back->setIcon(back_icon);

    ui->source_left->setText("");
    ui->source_left->setIconSize(size);
    ui->source_left->setIcon(left_icon);

    ui->source_forward->setText("");
    ui->source_forward->setIconSize(size);
    ui->source_forward->setIcon(forward_icon);

    ui->source_right->setText("");
    ui->source_right->setIconSize(size);
    ui->source_right->setIcon(right_icon);

    // object
    ui->object->setPixmap(object_icon);

    ui->object_up->setText("");
    ui->object_up->setIconSize(size);
    ui->object_up->setIcon(left_icon);

    ui->object_down->setText("");
    ui->object_down->setIconSize(size);
    ui->object_down->setIcon(right_icon);

    ui->object_left_rotate->setText("");
    ui->object_left_rotate->setIconSize(size);
    ui->object_left_rotate->setIcon(left_r_icon);

    ui->object_right_rotate->setText("");
    ui->object_right_rotate->setIconSize(size);
    ui->object_right_rotate->setIcon(right_r_icon);

    // reciever
    ui->reciever->setPixmap(reciever_icon);

    ui->reciever_back->setText("");
    ui->reciever_back->setIconSize(size);
    ui->reciever_back->setIcon(forward_icon);

    ui->reciever_left->setText("");
    ui->reciever_left->setIconSize(size);
    ui->reciever_left->setIcon(left_icon);

    ui->reciever_forward->setText("");
    ui->reciever_forward->setIconSize(size);
    ui->reciever_forward->setIcon(back_icon);

    ui->reciever_right->setText("");
    ui->reciever_right->setIconSize(size);
    ui->reciever_right->setIcon(right_icon);
}

//************* RECIEVER ****************//
// pressed reciver - start move
void Dialog::on_reciever_back_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit move(axes,~MAX_FREQUENCY);
}

void Dialog::on_reciever_right_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit move(axes,MAX_FREQUENCY);
}

void Dialog::on_reciever_forward_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit move(axes,MAX_FREQUENCY);
}

void Dialog::on_reciever_left_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit move(axes,~MAX_FREQUENCY);
}

// reciever realesed - stop move
void Dialog::on_reciever_forward_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit stop(axes);
}

// остановка движения вправо
void Dialog::on_reciever_right_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit stop(axes);
}

// остановка движения назад
void Dialog::on_reciever_back_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit stop(axes);
}

// остановка движения влево
void Dialog::on_reciever_left_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit stop(axes);
}

//************* OBJECT ****************//
// pressed object - start move
void Dialog::on_object_right_rotate_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit move(axes,MAX_FREQUENCY);
}

void Dialog::on_object_down_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit move_2(axes,MAX_FREQUENCY);
}

void Dialog::on_object_left_rotate_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit move(axes,~MAX_FREQUENCY);
}

void Dialog::on_object_up_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit move_2(axes,~MAX_FREQUENCY);
}

// object realesed - stop move
void Dialog::on_object_right_rotate_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit stop(axes);
}

void Dialog::on_object_down_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit stop_2(axes);
}

void Dialog::on_object_left_rotate_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit stop(axes);
}

void Dialog::on_object_up_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a2 = 1;
    emit stop_2(axes);
}

//************* SOURCE ****************//
// pressed source - start move

void Dialog::on_source_forward_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit move_2(axes,MAX_FREQUENCY);
}

void Dialog::on_source_right_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit move_2(axes,MAX_FREQUENCY);
}

void Dialog::on_source_back_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit move_2(axes,~MAX_FREQUENCY);
}

void Dialog::on_source_left_pressed()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit move_2(axes,~MAX_FREQUENCY);
}

// source realesed - stop move
void Dialog::on_source_forward_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit stop_2(axes);
}

void Dialog::on_source_right_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit stop_2(axes);
}

void Dialog::on_source_back_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a4 = 1;
    emit stop_2(axes);
}

void Dialog::on_source_left_released()
{
    Axes_Mask axes;
    axes = {0};
    axes.a1 = 1;
    emit stop_2(axes);
}












