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
#include "alphacam.h"
#include "rapeltechmed.h"
#include "stepmotor_rotate.h"
#include "myimageviewer.h"
#include "mygraphicsscene.h"
#include "definitions.h"
#include "tiff_image.h"
#include "dialog.h"
#include "dialog.h"



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    string qstr2str(QString x);

    QSettings *settingtxt;
    stepmotor_rotate * stepmotor, *stepmotor_2;
    AlphaCam * cam;
    RAPEltechMED * rap;
    QTimer * Timer;
    QString FileDirectory;
    myFrame * frame;
    tiff_image * tiff;
    myGraphicsScene * graphicsScene;
    QHostAddress Source, Destination;
    uint SourcePort , DestinationPort;
    Dialog *dialog;

    ushort NumberOfCoordinates, NumberOfImage, NumberOfShoot, CountOfShoot;

    int SizeOfStep, CoordinateRotate, Lenght_rotate_axe;
    int AccumulationTime;
    int min, max;
    int avFirstImage;

    bool status, XrayStatus;


signals:
    void getImage();
    void start_xray();
    void CompareCoordinates();
    void hist_changed();
    ushort* set_image(ushort*);
    QUdpSocket* set_UDP(QUdpSocket*);
    //int set_number_of_step(int);
    void nextStep(int,int);
    void finishAutoScan();
    void retry_acquire_image();

public slots:

    void myTimer();
    void finish_autoscan();

private slots:

    void onChangeU(uint u);

    void onChangeI(uint i);

    void onGetData(ushort * tdata);

    void on_Start_AutoScan_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_load_image_clicked();

    void on_convert_image_clicked();

    void convertTo8Bit();

    void convertToTiff();

    void chooseDirectory(uchar stage);

    void on_SaveAutoContrast_clicked();

    void on_LoadAutoContrast_clicked();

    void on_NumberOfSteps_textChanged(const QString &arg1);

    void on_with_rotate_stateChanged(int arg1);

    void on_handle_clicked();

    void make_shoot(uchar,uchar,int);

    void close_dialog();

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
