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

    ushort NumberOfCoordinates , NumberOfImage , NumberOfShoot , CountOfShoot;
    ushort * darkData;

    int SizeOfStep , CoordinateRotate , Lenght_rotate_axe;
    int AccumulationTime;
    int min , max;
    int avFirstImage;
    int step_size;

    QImage rxImage;

    bool status , XrayStatus;

    uchar waste;


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

private slots:

    void onChangeU(uint u);
    void onChangeI(uint i);
    void onGetData(ushort * tdata);
    void on_Start_AutoScan_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_load_image_clicked();
    void on_convert_image_clicked();
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
    void convertTo8Bit();
    void convertToTiff();
    void make_shoot(uchar , uchar , int);
    void close_dialog();
    void StartAutoScan();
    void MakeDarkImage();
    void MakeConfig();
    void SetUIData();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
