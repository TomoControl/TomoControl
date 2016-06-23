#ifndef PLCMWIDGET_H
#define PLCMWIDGET_H

#include <QDockWidget>
#include <QtSerialPort/QSerialPortInfo>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include "plcm.h"

namespace Ui {
class plcmwidget;
}

class plcmwidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit plcmwidget(QWidget *parent = 0);
    ~plcmwidget();
    PLCM *motor;
    bool *ok;

private slots:
    void on_connect_clicked();

    void on_init_clicked();

    void on_start_clicked();

    void on_stop_clicked();

    void on_move_clicked();

    void on_off_clicked();

private:
    Ui::plcmwidget *ui;
};

#endif // PLCMWIDGET_H
