#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDebug>
#include "definitions.h"
#include "structures.h"
#include <QVBoxLayout>
#include <QCameraImageCapture>
#include <QCamera>
#include <QLabel>
#include <QtMultimediaWidgets/QCameraViewfinder>
#include <QFileDialog>
#include "myimageviewer.h"




using namespace std;

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

    QCamera *Camera;
    QCameraViewfinder *CameraViewfinder;
    QCameraImageCapture *CameraImageCapture;
    QVBoxLayout *Layout;
    myFrame *frame;
    QLabel *lbl;
    bool stat1;
    ushort * imageData;

signals:
    void close_dialog();
    void move(Axes_Mask,int);
    void stop(Axes_Mask);
    void move_2(Axes_Mask,int);
    void stop_2(Axes_Mask);

    void make_shoot(uchar, uchar, int);
    void go(int,Axes_Mask);
    void go_2(int,Axes_Mask);
    void rap_off();

public slots:

    void set_icons();

    void set_image(ushort *);

private slots:

    void on_reciever_forward_released();

    void on_reciever_right_released();

    void on_reciever_back_released();

    void on_reciever_left_released();

    void on_pushButton_clicked();

    void on_reciever_back_pressed();

    void on_reciever_right_pressed();

    void on_reciever_forward_pressed();

    void on_reciever_left_pressed();

    void on_object_right_rotate_pressed();

    void on_object_down_pressed();

    void on_object_left_rotate_pressed();

    void on_object_up_pressed();

    void on_object_right_rotate_released();

    void on_object_down_released();

    void on_object_left_rotate_released();

    void on_object_up_released();

    void on_source_forward_pressed();

    void on_source_back_pressed();

    void on_source_left_pressed();

    void on_source_forward_released();

    void on_source_right_released();

    void on_source_back_released();

    void on_source_left_released();

    void on_source_right_pressed();

    void on_single_shoot_clicked();

    void on_webcam_on_stateChanged(int arg1);

    void on_source_left_clicked();

    void on_source_right_clicked();

    void on_source_forward_clicked();

    void on_source_back_clicked();

    void on_object_up_clicked();

    void on_object_down_clicked();

    void on_reciever_left_clicked();

    void on_reciever_right_clicked();

    void on_reciever_forward_clicked();

    void on_reciever_back_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
