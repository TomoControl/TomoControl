#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QGraphicsScene>
#include <QtGui>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QComboBox>
#include <QThread>

#include "alphacam.h"
#include "rapeltechmed.h"
#include "stepmotor_rotate.h"
#include "myimageviewer.h"
#include "mygraphicsscene.h"
#include "definitions.h"
#include "tiff_image.h"
#include "dialog.h"
#include "plcmwidget.h"
#include "mltcam.h"
#include "vivixcam.h"
#include "cam.h"
#include "service_functions.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSettings * settingtxt;
    QTimer * Timer;
    QThread * thread_1, * thread_2;
    QString FileDirectory;
    QHostAddress Source , Destination;
    uint SourcePort , DestinationPort;
    stepmotor_rotate * stepmotor_1, * stepmotor_2;
    cam *reciever;
    RAPEltechMED * rap;
    myFrame * frame;
    tiff_image * tiff;
    myGraphicsScene * graphicsScene;
    Dialog * dialog;
    plcmwidget * plcmwi;
    uchar selected_mode , calb_step , selected_cam , CountOfDarkImage;
    int compare , difference;
    int cent_1 , cent_2;

    short brCalMean;
    float calFactor;

    ushort NumberOfCoordinates , NumberOfImage , NumberOfShoot , CountOfShoot, CountOfFrame;
    ushort * darkData, *frameData, * brCalData;

    int SizeOfStep , CoordinateRotate , Lenght_rotate_axe;
    int AccumulationTime;
    int min , max;
    int avFirstImage;
    int step_size;

    QImage rxImage;

    bool status , XrayStatus, enable_continue;

    QTime start_time, finish_time;
    uchar waste;

    bool source_calibration_fg;
    int central_point[2];
    uchar source_calibration_step, counter_of_central_points;
    int difference_between_centals_point;

    //new Mode
    bool NewMode_fg;
    uchar ready;
    uint NewMode_range_source, NewMode_range_reciever;
    int NewMode_step;
    int NewMode_size_of_step_source, NewMode_size_of_step_reciever;
    ushort NewMode_current_step;
    ushort *addition_data;


signals:
    void getImage();
    void start_xray();
    void CompareCoordinates();
    void hist_changed();
    ushort* set_image(ushort*);
    QUdpSocket* set_UDP(QUdpSocket*);
    void nextStep(int,int);
    void finishAutoScan();
    void retry_acquire_image();
    void rap_off();
    void move_on(int, Axes_Mask);
    void finish();
    void init_stepmotor1(QHostAddress Source,
                         QHostAddress Destination,
                         uint SourcePort,
                         uint DestinationPort,
                         uchar ControlNum);
    void init_stepmotor2(QHostAddress Source, QHostAddress Destination, uint SourcePort, uint DestinationPort, uchar ControlNum);

public slots:

    void myTimer();
    void finish_autoscan();
    void source_calibration();
    void xray();
    void finish_calibration();
    void save_rap_working_time();
    void set_start_time();
    void set_finish_time();

private slots:

    void onChangeU(uint u);
    void onChangeI(uint i);
    void onGetData(ushort * tdata);
    void on_Start_AutoScan_clicked();
    void on_load_image_clicked();
    void on_SaveAutoContrast_clicked();
    void on_LoadAutoContrast_clicked();
    void on_NumberOfSteps_textChanged(const QString &arg1);
    void on_with_rotate_stateChanged(int arg1);
    void on_handle_clicked();
    void on_Calibrate_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_comboBox_2_currentIndexChanged(int index);
    void make_shoot(uchar , uchar , int);
    void close_dialog();
    void StartAutoScan();
    void MakeDarkImage();
    void MakeConfig();
    void SetUIData();

    void onCalibrationGetData(ushort * tdata);

    // new Mode
    void onNewModeGetData(ushort * tdata);
    void NewMode_continue();
    void NewMode_get_start_point();
    void NewMode_start_Xray();
    void NewMode_finish();
    void on_pushButton_5_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
