#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(QHostAddress)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // инициализация части переменных
    status = 0; // метка состояния сканирования: включено - 1 / выключено - 0
    XrayStatus = 0;
    CountOfShoot = 0;
    CountOfDarkImage = 0;
    CountOfFrame = 0;
    selected_mode = 0;
    selected_cam = 0;
    calb_step = 0;
    difference = 0;
    step_size = 5000;
    cent_2 = 0;
    enable_continue = true;

    source_calibration_fg = 0;
    source_calibration_step = 0;
    counter_of_central_points = 0;
    central_point[0] = 0;
    central_point[1] = 0;
    difference_between_centals_point = 0;

    QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_Indexed8);
    rxImage = image;

    start_time.setHMS(0,0,0,0);
    finish_time.setHMS(0,0,0,0);

    // графический интерфейс
    ui->setupUi(this);
    ui->with_rotate->setChecked(true);

    tiff = new tiff_image;

    dialog = new Dialog;
    plcmwi = new plcmwidget;

    // создание поля для изображения
    frame = new myFrame;
    ui->frame_3->setLayout(new QHBoxLayout);
    ui->frame_3->layout()->addWidget(frame);

    // создание гистограммы
    graphicsScene = new myGraphicsScene(ui->graphicsView->width(), ui->graphicsView->height());
    ui->graphicsView->setScene(graphicsScene);

    // инициализация драйверов ШД
    stepmotor_1 = new stepmotor_rotate;
    stepmotor_2 = new stepmotor_rotate;

    // threads
    thread_1 = new QThread;
    thread_2 = new QThread;

    qRegisterMetaType <QHostAddress> ("QHostAddress");
    qRegisterMetaType <Axes_Mask> ("Axes_Mask");

    connect(stepmotor_1, SIGNAL(finished()), thread_1, SLOT(terminate()));
    connect(stepmotor_2, SIGNAL(finished()), thread_2, SLOT(terminate()));
    connect(this,
            SIGNAL(init_stepmotor1(QHostAddress,QHostAddress,uint,uint,uchar)),
            stepmotor_1,
            SLOT(initialization(QHostAddress,QHostAddress,uint,uint,uchar)));
    connect(this,
            SIGNAL(init_stepmotor2(QHostAddress,QHostAddress,uint,uint,uchar)),
            stepmotor_2,
            SLOT(initialization(QHostAddress,QHostAddress,uint,uint,uchar)));


    stepmotor_1->moveToThread(thread_1);
    stepmotor_2->moveToThread(thread_2);

    stepmotor_1->setRunning(true);
    stepmotor_2->setRunning(true);

    thread_1->start(QThread::TimeCriticalPriority);
    thread_2->start(QThread::TimeCriticalPriority);

    Source = ("192.168.10.1");//QHostAddress::LocalHost;
    SourcePort = 1075;
    Destination = ("192.168.10.10");//QHostAddress::LocalHost;
    DestinationPort = 5000;
    uchar ControlNum;
    ControlNum = 1;
    emit init_stepmotor1(Source, Destination, SourcePort, DestinationPort, ControlNum);
    //stepmotor_1->initialization(Source, Destination, SourcePort, DestinationPort, ControlNum);
    SourcePort = 1234;
    Destination = ("192.168.10.11");//QHostAddress::LocalHost;
    ControlNum = 2;
    emit init_stepmotor2(Source, Destination, SourcePort, DestinationPort, ControlNum);
    //stepmotor_2->initialization(Source, Destination, SourcePort, DestinationPort, ControlNum);

    Timer = new QTimer;
    QObject::connect(Timer , SIGNAL(timeout()) , this, SLOT(myTimer()));
    Timer->start(200);

    ui->Start_AutoScan->setDisabled(true);
    ui->handle->setDisabled(true);
    ui->TimeCorrect->setDisabled(true);
    ui->Exposure->setDisabled(true);
    ui->Compare->setDisabled(true);
    ui->I_Auto->setDisabled(true);
    ui->U_Auto->setDisabled(true);
    ui->NumberOfSteps->setDisabled(true);

    // инициализация источника РИ
    rap = new RAPEltechMED;
    rap->initialization();

    // установление соединений
    connect(frame, SIGNAL(histCalculated(ushort*)), graphicsScene, SLOT(onHistCalculated(ushort*)));
    connect(graphicsScene, SIGNAL(changeHistOutput(int,int)), frame, SLOT(onChangeHistogrammWidget(int,int)));

    // определение времени работы источника РИ
    connect(rap,SIGNAL(xrayOn()),this,SLOT(set_start_time()));
    connect(rap,SIGNAL(xrayOff()),this,SLOT(set_finish_time()));

    //new Mode
    ready = 0;
    NewMode_fg = 0;
    NewMode_range = 0;
    NewMode_step = 0;
    NewMode_size_of_step = 0;
    NewMode_current_step = 0;
}

MainWindow::~MainWindow()
{
    // разрыв соединений
    disconnect(frame, SIGNAL(histCalculated(ushort*)), graphicsScene, SLOT(onHistCalculated(ushort*)));
    disconnect(graphicsScene, SIGNAL(changeHistOutput(int,int)), frame, SLOT(onChangeHistogrammWidget(int,int)));

    QSettings *setting = new QSettings ( QDir::currentPath() + "LastValue.ini" , QSettings::IniFormat);
    setting->setValue("NumberOfImage" , ui->NumberOfSteps->text().toInt());
    setting->setValue("Current" , (uchar)ui->I_Auto->text().toShort());
    setting->setValue("Voltage" , (uchar)ui->U_Auto->text().toShort());
    setting->setValue("Exposure" , ui->Exposure->text().toInt());
    setting->setValue("IntensiveCorrection" , ui->Compare->text().toInt());
    setting->setValue("TimeCorrect" , ui->TimeCorrect->text().toInt());
    setting->setValue("With_rotate" , ui->with_rotate->isChecked()); // TODO: Check number of shoot
    setting->setValue("Any_image" , ui->any_image->isChecked());
    setting->setValue("BrCalibration" , ui->brCalibration->isChecked());
    setting->sync();

    stepmotor_1->setRunning(false);
    stepmotor_2->setRunning(false);

    // отключение и осовобождение памяти для драйверов ШД
    Timer->stop();
    QObject::disconnect(Timer , SIGNAL(timeout()) , this , SLOT(myTimer()));
    stepmotor_1->go_emergency();
    stepmotor_2->go_emergency();

    delete stepmotor_1;
    delete stepmotor_2;

    delete thread_1;
    delete thread_2;
    delete Timer;


    // отключение и осовобождение памяти для приемника РИ
    if(ui->comboBox_2->currentIndex() <= (NUMBER_OF_RECIEVER - 1))
    {
        reciever->Disconnect();
        delete reciever;
    }

    // отключение и осовобождение памяти для источника РИ
    rap->ClosePort();

    delete rap;
    delete frame;
    delete graphicsScene;
    delete ui;
}

void MainWindow::myTimer()    //действие по таймеру
{
    // отображение текущей координаты ШД
    show_current_position show, show_2;

    show = stepmotor_1->get_current_position();
    show_2 = stepmotor_2->get_current_position();

    show.Position_1 /= 640; // TODO корректный перевод в мм
    show.Position_2 /= 640;
    show.Position_3 /= 640;

    //show_2.Position_1 /= 640; //вращение
    show_2.Position_2 /= 640;
    show_2.Position_3 /= 640;


    ui->pos1->setText(QString::number(show.Position_1));
    ui->pos2->setText(QString::number(show.Position_2));
    ui->pos3->setText(QString::number(show.Position_3));

    ui->pos4->setText(QString::number(show_2.Position_1));
    ui->pos5->setText(QString::number(show_2.Position_2));
    ui->pos6->setText(QString::number(show_2.Position_3));

    // отображение текущего шага сканирования
    ui->current_step->setText(QString::number(CountOfShoot));

    // отображение ошибок
    //show_errors(); TODO индикация ошибок
}

// отображение текущего значения напряжения на РТ
void MainWindow::onChangeU(uint u)
{
    ui->Current_U->setNum((int)u);
}
// отображение текущего значения тока РТ
void MainWindow::onChangeI(uint i)
{
    ui->Current_I->setNum((int)i);
}

// ******************** Ручное управление системой ********************* //
// активация диалогового окна с возможностью ручных перемещений
void MainWindow::on_handle_clicked()
{
    selected_mode = 3;
    connect(dialog,SIGNAL(close_dialog()),this,SLOT(close_dialog()));
    connect(dialog,SIGNAL(make_shoot(uchar,uchar,int)),this,SLOT(make_shoot(uchar,uchar,int)));
    connect(dialog,SIGNAL(move(Axes_Mask,int)),stepmotor_1,SLOT(manual_movement(Axes_Mask,int)));
    connect(dialog,SIGNAL(stop(Axes_Mask)),stepmotor_1,SLOT(stop_movement(Axes_Mask)));
    connect(dialog,SIGNAL(move_2(Axes_Mask,int)),stepmotor_2,SLOT(manual_movement(Axes_Mask,int)));
    connect(dialog,SIGNAL(stop_2(Axes_Mask)),stepmotor_2,SLOT(stop_movement(Axes_Mask)));
    connect(dialog,SIGNAL(go(int,Axes_Mask)),stepmotor_1,SLOT(go_to(int,Axes_Mask)));
    connect(dialog,SIGNAL(go_2(int,Axes_Mask)),stepmotor_2,SLOT(go_to(int,Axes_Mask)));

    if(!ui->xray_signal->isChecked())
    {
        connect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
        connect(dialog,SIGNAL(rap_off()),rap,SLOT(off()));
    }
    connect(reciever, SIGNAL(GetDataComplete(ushort*)),dialog,SLOT(set_image(ushort*)));
    connect(reciever, SIGNAL(GetDataComplete(ushort*)),this,SLOT(onGetData(ushort*)));

    dialog->set_icons();
    dialog->show();
}

void MainWindow::make_shoot(uchar U, uchar I, int time)
{
    reciever->SetAccumulationTime(time);
    if(!ui->xray_signal->isChecked())
    {
       rap->on(U, I);
    }
    else reciever->AcquireImage();
}

// деактивация диалогового окна
void MainWindow::close_dialog()
{
    selected_mode = 0;
    disconnect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
    disconnect(dialog,SIGNAL(make_shoot(uchar,uchar,int)),this,SLOT(make_shoot(uchar,uchar,int)));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)),dialog,SLOT(set_image(ushort*)));
    disconnect(dialog,SIGNAL(close_dialog()),this,SLOT(close_dialog()));
    disconnect(dialog,SIGNAL(move(Axes_Mask,int)),stepmotor_1,SLOT(manual_movement(Axes_Mask,int)));
    disconnect(dialog,SIGNAL(stop(Axes_Mask)),stepmotor_1,SLOT(stop_movement(Axes_Mask)));
    disconnect(dialog,SIGNAL(move_2(Axes_Mask,int)),stepmotor_2,SLOT(manual_movement(Axes_Mask,int)));
    disconnect(dialog,SIGNAL(stop_2(Axes_Mask)),stepmotor_2,SLOT(stop_movement(Axes_Mask)));
    disconnect(dialog,SIGNAL(go(int,Axes_Mask)),stepmotor_1,SLOT(go_to(int,Axes_Mask)));
    disconnect(dialog,SIGNAL(go_2(int,Axes_Mask)),stepmotor_2,SLOT(go_to(int,Axes_Mask)));
    disconnect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)),dialog,SLOT(set_image(ushort*)));
    disconnect(dialog,SIGNAL(rap_off()),rap,SLOT(off()));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)),this,SLOT(onGetData(ushort*)));
    dialog->hide();
}


// ******************** Автоматическое управление системой ********************* //

// Запуск автоматического набора проекционных данных
void MainWindow::on_Start_AutoScan_clicked()
{
    if(!status)
    {
        FileDirectory = QString("AutoScan/%1/%2/").arg(QDate::currentDate().toString("yyyy-MM-dd"))
                                                 .arg(QTime::currentTime().toString("hh-mm-ss"));

        selected_mode = 1;
        // желаемое количество проекций
        NumberOfImage = ui->NumberOfSteps->text().toInt();

        // индикация процесса выполнения набора проекций
        ui->progressBar->setRange(0,NumberOfImage);



        // расчет длины 1 шага при перемещении объекта
        if ((FULL_TURN % NumberOfImage) == 0)
        {
            SizeOfStep = FULL_TURN / NumberOfImage;
            qDebug() << "size of step" << SizeOfStep << FULL_TURN << NumberOfImage;
            ui->ErrorLabel->setText("");
        }
        else
        {
            ui->ErrorLabel->setText("Введите корректное число проекций");
            return;
        }

        // установление соединений для автосканирования
        if(!ui->xray_signal->isChecked())
        {
            connect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
            connect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
            connect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));
        }
        connect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onGetData(ushort *)));
        connect(this, SIGNAL(nextStep(int,int)), stepmotor_1, SLOT(calculate_go(int,int)));
        connect(stepmotor_1, SIGNAL(continue_scan()), reciever, SLOT(AcquireImage()));
        connect(this, SIGNAL(finishAutoScan()), this, SLOT(finish_autoscan()));
        connect(this,SIGNAL(retry_acquire_image()),reciever,SLOT(AcquireImage()));

        status = 1;
        CountOfShoot = 0;
        ui->Start_AutoScan->setText("Stop AutoScan");

        // установка времени экспозиции камеры
        AccumulationTime = ui->Exposure->text().toInt();

        // определение необходимости раскадровки снимков
        if(ui->any_image->isChecked())
        {
            enable_continue = false;
            AccumulationTime /= NUMBER_OF_FRAME;
        }

        reciever->SetAccumulationTime(AccumulationTime);

        if ((ui->comboBox_2->currentIndex() == 1)&&(NUMBER_OF_DARK_IMAGE > 0))
        {
            MakeDarkImage();
        }
        else
        {
            StartAutoScan();
        }
    }
    else
    {
        finish_autoscan();
    }
}

void MainWindow::MakeDarkImage()
{
    selected_mode = 4;
    reciever->AcquireImage();
}

void MainWindow::StartAutoScan()
{
    if(!ui->xray_signal->isChecked())
    {
        rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
    }
    else reciever->AcquireImage();
}

void MainWindow::finish_autoscan()
{
    // отключение соединений, необходимых для автосканирования
    disconnect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
    disconnect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
    disconnect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onGetData(ushort *)));
    disconnect(this, SIGNAL(nextStep(int,int)), stepmotor_1, SLOT(calculate_go(int,int)));
    disconnect(stepmotor_1, SIGNAL(continue_scan()), reciever, SLOT(AcquireImage()));
    disconnect(this, SIGNAL(finishAutoScan()), rap, SLOT(off()));
    disconnect(this,SIGNAL(retry_acquire_image()),reciever,SLOT(AcquireImage()));

    // выключение источника, сброс индикации
    rap->off();
    status = 0;
    selected_mode = 0;
    CountOfShoot = 0;
    ui->Start_AutoScan->setText("Start AutoScan");
}

// прием изображения от камеры
void MainWindow::onGetData(ushort * tdata)
{
        qDebug() << "OnGetData";
        ushort * dData;
        dData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
        memcpy(dData, tdata, IMAGE_WIDTH * IMAGE_HEIGHT * 2);

        switch (selected_mode)
        {
        case 1:
        {
            if ((CountOfShoot == 0) && (CountOfFrame == 0))
            {
                qDebug() << "mainwindow :: create ini";
                calFactor = 1.0;

                // создание ini-файла с параметрами съемки
                QString Time_start = QTime::currentTime().toString("hh-mm-ss");
                QSettings *setting = new QSettings ( FileDirectory + "ShootingMode.ini" , QSettings::IniFormat);
                setting->setValue("NumberOfImage" , NumberOfImage);
                setting->setValue("Current" , (uchar)ui->I_Auto->text().toShort());
                setting->setValue("Voltage" , (uchar)ui->U_Auto->text().toShort());
                setting->setValue("StartTime" , Time_start);
                setting->sync();
                MakeConfig(); // конфигурационный файл для восстановления проекций
                settingtxt = new QSettings ( FileDirectory + "txt.ini" , QSettings::IniFormat);

                // Загружаем калибровочный файл, только если установлен флаг
                if (ui->brCalibration->isChecked())
                {
                    brCalData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
                    brCalMean = 0;


                    QFile brCalFile("C:/TomoControl/brCal.raw");
                    if (!brCalFile.open(QIODevice::ReadOnly))
                    {
                        qDebug() << "Конвертирование::Ошибка открытия файла";
                    }
                    brCalFile.read((char*)brCalData, IMAGE_WIDTH*IMAGE_HEIGHT*2);

                    // Вычисляем максимальное значение по файлу
                    brCalMean = 0;

                    for (int i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; i++)
                        if (brCalData[i] > brCalMean) brCalMean = brCalData[i];
                    qDebug() << brCalMean;
                }
            }

            if((ui->any_image->isChecked()) && (CountOfFrame == 0))
            {
                frameData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
            }

            // реализация заданного числа кадров в снимке
            if(!enable_continue && ui->any_image->isChecked())
            {
                for (int k = 0; k < IMAGE_HEIGHT - 1; k++)
                {
                    for (int j = 0; j < IMAGE_WIDTH - 1; j++)
                    {
                        frameData[(k * IMAGE_WIDTH) + j] += dData[(k * IMAGE_WIDTH) + j];
                    }
                }

                if( CountOfFrame == (NUMBER_OF_FRAME - 1))
                {
                    enable_continue = true;
                    memcpy(dData, frameData, IMAGE_WIDTH * IMAGE_HEIGHT * 2);
                    delete[] frameData;
                }
                else
                {
                    CountOfFrame ++;
                    reciever->AcquireImage();
                    delete[] dData;
                    return;
                }
            }

            int avpixel, pixel;
            avpixel = dData[0];
            pixel = 0;

            // расчет среднего значения пиксела
            for (int k = 0; k < 50; k++)
            {
                for (int  j = 0; j < 50; j++)
                {
                    pixel = dData[(k * IMAGE_WIDTH) + j];
                    avpixel = (avpixel + pixel )/ 2;
                }
            }
            qDebug() << "avpixel = " << avpixel;

            int needRenew = 0;

            if (CountOfShoot == 0)
            {
                avFirstImage = avpixel;
            }
            else
            {
                // коррекция времени экспозиции
                if (avpixel - avFirstImage > ui->Compare->text().toInt())
                {
                    AccumulationTime += ui->TimeCorrect->text().toInt();
                    reciever->SetAccumulationTime(AccumulationTime);
                    needRenew = 1;
                }
                else if (avpixel - avFirstImage < (-1*ui->Compare->text().toInt()))
                {
                    AccumulationTime -= ui->TimeCorrect->text().toInt();
                    reciever->SetAccumulationTime(AccumulationTime);
                    needRenew = 1;
                }

                if (needRenew)
                {
                    qDebug() << "Коррекция экспозиции: Первый снимок - "
                             << avFirstImage
                             << " Текущий снимок - "
                             << avpixel
                             << " Скорректированное время накопления - "
                             << AccumulationTime;
                }
            }

            qDebug() << "Cканирование:: Рентгеновский снимок" << CountOfShoot + 1 << "из" << NumberOfImage;
            qDebug() << "Cканирование:: Получение снимка завершено";


            if ((!needRenew))
            {
                // сохранение данных о снимке
                QString txt = QString("Image%1").arg(CountOfShoot);
                QString value = QString("Average = %1 Accumulation time = %2").arg(avpixel);
                settingtxt->setValue(txt,value);
                settingtxt->sync();

                // сохранение изображений в tiff-формате

                // Коррекция первого файла по автоматически вычисленным границам
                for (int k = 0; k < IMAGE_HEIGHT - 1; k++)
                {
                    for (int j = 0; j < IMAGE_WIDTH - 1; j++)
                    {
                        if (ui->brCalibration->isChecked())
                            calFactor = (float)brCalMean/(float)brCalData[(k * IMAGE_WIDTH) + j];
                        //qDebug() << calFactor;
                        pixel = dData[(k * IMAGE_WIDTH)+j] * calFactor;
                        //pixel = 65535  * (pixel - min) / (max - min) ;
                        if (ui->comboBox_2->currentIndex() != 0) dData[(k * IMAGE_WIDTH) + j] = pixel;
                        else dData[(k * IMAGE_WIDTH) + j] = 65535 - pixel;
                    }
                }

                QString NameForSaveImage;
                NameForSaveImage = service_functions::RenameOfImages(CountOfShoot-1);
                QString file = FileDirectory + NameForSaveImage;

                tiff->WriteTIFF(file.toStdString(), dData, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 1., 16);
                frame->setRAWImage(dData);
                CountOfShoot ++;
                CountOfFrame = 0;
                enable_continue = false;
            }

            delete[] dData;
            ui->progressBar->setValue(CountOfShoot);

            if (CountOfShoot == NumberOfImage)
            {
                emit finishAutoScan();
                return;
            }
            emit nextStep(SizeOfStep, CountOfShoot);
            qDebug() << "nextStep" << SizeOfStep << CountOfShoot;
            break;
        }

        case 2:
        {
            calb_step ++;
            qDebug() << "move_solution";

            //frame->setRAWImage(dData);

            int left_border = 0, right_border = 0, center = 0;
            bool left_c = 1, right_c = 1;

            int i = 600 ;
            int pixel = dData[i * IMAGE_WIDTH];
            int pixel_old = dData[i * IMAGE_WIDTH];

            for (int j = 100; j < IMAGE_WIDTH - 1; j++ )
            {
                pixel_old = pixel;
                pixel = dData[i * IMAGE_WIDTH + j];

                if ((pixel - pixel_old > THRESHOLD)&&(left_c))
                {
                    qDebug() << "left" << j;
                    left_border = j;
                    left_c = 0;
                }
                if ((-1*(pixel - pixel_old) > THRESHOLD)&&(right_c)&&(!left_c))
                {
                    qDebug() << "right" << j;
                    right_border = j;
                    right_c = 0;
                }
                if(left_border*right_border > 0)
                {
                    qDebug() <<" zawel";
                    center = (right_border + left_border) / 2;
                    left_border = 0;
                    right_border = 0;
                    if (calb_step == 1) {compare = center; qDebug() << "compare" << compare; cent_1 = center;}
                    if (calb_step == 2)
                    {
                        difference = (compare - center);
                        cent_2 = center;
                        qDebug() << "cent" << cent_1 << cent_2;
                    }
                }
            }
            delete []dData;

            qDebug() << calb_step << "step";
            switch (calb_step)
            {
            case 1:
            {
                qDebug() << "calb step = 1";
                int step;
                Axes_Mask axes;
                axes = stepmotor_2->reset_axes_mask();
                axes.a4 = 1;
                step = 10000;
                stepmotor_2->go_to_for_calb(step,axes);
                break;
            }
            case 2:
            {
                qDebug() << "calb step = 2";
                int step;
                Axes_Mask axes;
                axes = stepmotor_2->reset_axes_mask();
                axes.a1 = 1;

                if(difference > 5 )
                {step = ~step_size; qDebug() << difference << "difference";}
                if(difference < -5 ) {step = step_size;qDebug() << difference << "difference_22222";}
                stepmotor_2->go_to_for_calb(step,axes);

                // возвращение на первую линию
                step = ~10000;
                axes = stepmotor_2->reset_axes_mask();
                axes.a4 = 1;
                stepmotor_2->go_to(step,axes);
                calb_step = 0;
                break;
            }
            default:
                break;
            }

            qDebug() << difference << "dif " << cent_2 << "cent_2";
            if((difference < 5)&&(-1*difference < 5)&&(cent_2 != 0))
            {
                difference = 0;
                qDebug() << "finish";
                emit finish();
                return;
            }

            if((calb_step == 0) && (difference > 5))
            {
                qDebug() << "calb diff";
                //difference = 0;
                cent_1 = 0;
                cent_2 = 0;
                step_size /= 2;
                reciever->AcquireImage();
            }


            break;
        }
        case 3:
            //frame->setRAWImage(dData);
            rap->off();
            break;
        case 4:
        {
            CountOfDarkImage ++;
            QString NameForSaveImage;
            NameForSaveImage = QString("DarkImage%1.raw").arg(CountOfDarkImage);
            QFile file(FileDirectory + NameForSaveImage);
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "Сканирование:: Сохранение темнового изображения:: Ошибка открытия файла";
            }
            file.write((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);
            if (CountOfDarkImage < NUMBER_OF_DARK_IMAGE)
            {
                qDebug() << "next dark";
                reciever->AcquireImage();
            }
            else
            {
                qDebug() << "else dark";

                selected_mode = 1;
                CountOfDarkImage = 0;
                StartAutoScan();
                delete darkData;
            }
            break;
        }
        default:
            break;  
        }
}


// загрузка полученных ранее проекционных данных
void MainWindow::on_load_image_clicked()
{
    ui->comboBox->clear();
    uchar CountOfImage;
    QSettings *setting_stop = new QSettings ( FileDirectory, QSettings::IniFormat );
    CountOfImage = setting_stop->value("NumberOfImage" , 0).toInt();
    for(uint i = 1; i <= CountOfImage; i++)
    { // подгрузка в комбобокc при автосканировании
        QString CheckedImage;
        if (i < 9) CheckedImage = QString("image_000%1.raw").arg(i);
        if (i >=9 && i < 99) CheckedImage = QString("image_00%1.raw").arg(i);
        if (i >=99) CheckedImage = QString("image_0%1.raw").arg(i);
        ui->comboBox->addItem(CheckedImage);
    }  
}

void MainWindow::on_SaveAutoContrast_clicked()
{
//    chooseDirectory(4);
//    FileDirectory.remove("ShootingMode.ini");
//    FileDirectory += "AutoContrast.ini";
//    QSettings *setting = new QSettings ( FileDirectory , QSettings::IniFormat);
//    setting->setValue("left_limit" , 270 - graphicsScene->lineLow->rect().width());
//    setting->setValue("right_limit" , graphicsScene->lineHigh->scenePos().x());
//    setting->setValue("height" , graphicsScene->lineLow->rect().height());
//    setting->setValue("width" , graphicsScene->lineLow->rect().width());
//    chooseDirectory(2);

    plcmwi->show();
}

void MainWindow::on_LoadAutoContrast_clicked()
{
    QSettings *setting = new QSettings ( FileDirectory , QSettings::IniFormat);
    uchar posLow , posHigh , height , width;
    posLow = setting->value("left_limit").toInt();
    posHigh = setting->value("right_limit").toInt();
    height = setting->value("height").toInt();
    width = setting->value("width").toInt();
    qDebug() << posLow << posHigh;
    int x1,x2;
    x1 = BITS * width/(270-6);
    x2 = BITS * posHigh/(270-6);
    frame->onChangeHistogrammWidget(x1,x2);
}

// проверка корректности ввода желаемого числа проекций
void MainWindow::on_NumberOfSteps_textChanged(const QString &arg1)
{
    (QString)waste = arg1;
    if ((ui->NumberOfSteps->text().toInt() == 1 && ui->with_rotate->isChecked()) ||
        (ui->NumberOfSteps->text().toInt() != 1 && !ui->with_rotate->isChecked()) ||
         ui->NumberOfSteps->text().toInt() < 1 || ui->comboBox_2->currentIndex() > NUMBER_OF_RECIEVER - 1)
        ui->Start_AutoScan->setDisabled(true);
    else
        ui->Start_AutoScan->setDisabled(false);
}

//????
void MainWindow::on_with_rotate_stateChanged(int arg1)
{
    waste = arg1;
    if ((ui->NumberOfSteps->text().toInt() == 1 && ui->with_rotate->isChecked()) ||
        (ui->NumberOfSteps->text().toInt() != 1 && !ui->with_rotate->isChecked()) ||
         ui->NumberOfSteps->text().toInt() < 1 || ui->comboBox_2->currentIndex() > NUMBER_OF_RECIEVER - 1)
        ui->Start_AutoScan->setDisabled(true);
    else
        ui->Start_AutoScan->setDisabled(false);
}

// калибровка системы
void MainWindow::on_Calibrate_clicked()
{
    uchar AxeOfCalb_1;
    uchar AxeOfCalb_2;
    uchar AxeOfCalb_3;
    AxeOfCalb_1 = 1;
    AxeOfCalb_2 = 1;
    AxeOfCalb_3 = 0;
    stepmotor_1->setCalibrAxe(AxeOfCalb_1 , AxeOfCalb_2 , AxeOfCalb_3);
    AxeOfCalb_3 = 1;
    stepmotor_2->setCalibrAxe(AxeOfCalb_1 , AxeOfCalb_2 , AxeOfCalb_3);
    stepmotor_1->calibrate();
    stepmotor_2->calibrate();
}


void MainWindow::source_calibration()
{
    if(!status)
    {
        // установление соединений для автосканирования
        connect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
        connect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onGetData(ushort *)));
        connect(stepmotor_2,SIGNAL(continue_scan()),reciever,SLOT(AcquireImage()));
        connect(this,SIGNAL(finish()),this,SLOT(finish_calibration()));
        selected_mode  = 2;
        status = 1;

        ui->pushButton_3->setText("Stop");

        // установка времени экспозиции камеры
        AccumulationTime = ui->Exposure->text().toInt();
        reciever->SetAccumulationTime(AccumulationTime);
        // включение источника РИ
//        rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
        xray();
    }
    else
    {
        finish_calibration();
    }
}

void MainWindow::xray()
{
    rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
}

void MainWindow::finish_calibration()
{
    rap->off();
    disconnect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onCalibrationGetData(ushort *)));
    disconnect(stepmotor_2,SIGNAL(continue_scan()),reciever,SLOT(AcquireImage()));
    disconnect(stepmotor_2,SIGNAL(continue_scan()),this,SLOT(xray()));

    disconnect(this,SIGNAL(finish()),this,SLOT(finish_calibration()));
    disconnect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
    disconnect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));

    // выключение источника, сброс индикации
    status = 0;
    ui->pushButton_2->setText("Start");
    //selected_mode = 0;
}




void MainWindow::on_pushButton_3_clicked()
{
    source_calibration();
}

void MainWindow::on_pushButton_4_clicked()
{
    rap->off();
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    ui->comboBox_2->removeItem(3);
    ui->comboBox_2->repaint();
    switch (index)
    {
    case 0:
        qDebug() << "text0" << ui->comboBox_2->currentText();
        reciever = new AlphaCam;
        //selected_reciever = 1;
        break;
    case 1:
        qDebug() << "text1" << ui->comboBox_2->currentText();
        reciever = new MLTCam;
        darkData = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
        //selected_cam = 2;
        break;
    case 2:
        qDebug() << "text2" << ui->comboBox_2->currentText();
        reciever = ViVIXCam::getInstance();
        reciever->InitializationCam();

    default:
        break;
    }
    SetUIData();
}

void MainWindow::SetUIData()
{
    QSettings *setting = new QSettings ( QDir::currentPath() + "LastValue.ini" , QSettings::IniFormat);
    ui->NumberOfSteps->setText(setting->value("NumberOfImage").toString());
    ui->U_Auto->setText(setting->value("Voltage").toString());
    ui->I_Auto->setText(setting->value("Current").toString());
    ui->Compare->setText(setting->value("IntensiveCorrection").toString());
    ui->TimeCorrect->setText(setting->value("TimeCorrect").toString());
    ui->Exposure->setText(setting->value("Exposure").toString());
    ui->with_rotate->setChecked(setting->value("With_rotate").toBool());
    ui->any_image->setChecked(setting->value("Any_image").toBool());
    ui->brCalibration->setChecked(setting->value("BrCalibration").toBool());
    ui->comboBox_2->setDisabled(true);
    ui->Start_AutoScan->setDisabled(false);
    ui->handle->setDisabled(false);
    ui->TimeCorrect->setDisabled(false);
    ui->Exposure->setDisabled(false);
    ui->Compare->setDisabled(false);
    ui->I_Auto->setDisabled(false);
    ui->U_Auto->setDisabled(false);
    ui->NumberOfSteps->setDisabled(false);
}

void MainWindow::MakeConfig()
{
    int ObjectToSource = 119;
    int CameraToSource = 498;

    int PixelSize;
    if (ui->comboBox_2->currentIndex() == 0) // alphaCam
        PixelSize = 189;
    if (ui->comboBox_2->currentIndex() == 1) // mltCam
        PixelSize = 50;
    if (ui->comboBox_2->currentIndex() == 2) // ViVixCam
        PixelSize = 200;

    float ScaledPixelSize = (float)ObjectToSource*(float)PixelSize/(float)CameraToSource;
    qDebug() << ScaledPixelSize;

    QSettings *setting = new QSettings ( FileDirectory + "s_.log" , QSettings::IniFormat);
    setting->setValue("System/Scanner" , "MEVLINDET-1");
    setting->setValue("System/Instrument S/N" , "10H03060");
    setting->setValue("System/Software" , "Version 1. 1 (build 3)");
    setting->setValue("System/Home Directory" , "C:\\");
    setting->setValue("System/Source Type" , "RTW 60/100");
    setting->setValue("System/Camera" , "SHT MR285MC");
    setting->setValue("System/Camera Pixel Size (um)" , QString::number(PixelSize));
    setting->setValue("System/CameraXYRatio" , QString::number(1.0));

    setting->setValue("Acquisition/Filename Prefix" , "s_");
    setting->setValue("Acquisition/Number Of Files" , QString::number(ui->NumberOfSteps->text().toInt()));
    setting->setValue("Acquisition/Number Of Rows" , QString::number(IMAGE_HEIGHT));
    setting->setValue("Acquisition/Number Of Columns" , QString::number(IMAGE_WIDTH));
    setting->setValue("Acquisition/Optical Axis (line)" , QString::number(1000));
    setting->setValue("Acquisition/Object to Source (mm)" , QString::number(ObjectToSource));
    setting->setValue("Acquisition/Camera to Source" , QString::number(CameraToSource));

    setting->setValue("Acquisition/Source Voltage (kV)" , QString::number(ui->U_Auto->text().toInt()));
    setting->setValue("Acquisition/Source Current (uA)" , QString::number(ui->I_Auto->text().toInt()));
    setting->setValue("Acquisition/Image Pixel Size (um)" , QString::number(PixelSize));
    setting->setValue("Acquisition/Scaled Image Pixel Size (um)" , QString::number(int(ScaledPixelSize*100)/100));
    setting->setValue("Acquisition/Image Format" , "TIFF");
    setting->setValue("Acquisition/Depth (bits)" , QString::number(16));
    setting->setValue("Acquisition/Screen LUT" , "ScreenLUT");
    setting->setValue("Acquisition/Exposure(ms)" , QString::number(ui->Exposure->text().toInt()));
    setting->setValue("Acquisition/Rotation Step (deg)" , QString::number(360.0/(float)ui->NumberOfSteps->text().toInt()));
    setting->setValue("Acquisition/Use 360 Rotation" , "YES");
    setting->setValue("Acquisition/Flat Field Correction" ,"OFF" );
    setting->setValue("Acquisition/Sharpening (%)" , "Sharpening");
    setting->setValue("Acquisition/Geometrical Correction" , "ON" );
    setting->setValue("Acquisition/Rotation Direction" , "CC");
    setting->setValue("Acquisition/Type of Detector Motion" , "STEP AND SHOOT");
    setting->sync();

    service_functions::deletespace(FileDirectory + "s_.log");
}

void MainWindow::save_rap_working_time()
{
//    QFile file("D:/QtProjects/s_.log"); TODO: do it
//    QTime full_time;
//    if (file.exists())
//    {
//        file.close();
//        QSettings *setting = new QSettings ( "C:/TomoControl/s_.log" , QSettings::IniFormat);
//        full_time = setting->value("work time").toTime();
//    }
//    QSettings *setting_2 = new QSettings ( "C:/TomoControl/s_.log" , QSettings::IniFormat);
//    full_time += (finish_time - start_time);
//    setting_2->setValue("work time",full_time);
}

void MainWindow::set_start_time()
{
    start_time = QTime::currentTime();
}

void MainWindow::set_finish_time()
{
    finish_time = QTime::currentTime();
    save_rap_working_time();
}

// Source calibraion
void MainWindow::on_pushButton_2_clicked()
{
    if(!source_calibration_fg)
    {
        // установление соединений для калибровки
        connect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
        connect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onCalibrationGetData(ushort *)));
//        connect(stepmotor_2,SIGNAL(continue_scan()),reciever,SLOT(AcquireImage()));
        connect(this,SIGNAL(finish()),this,SLOT(finish_calibration()));
        connect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
        connect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));
        connect(stepmotor_2,SIGNAL(continue_scan()),this,SLOT(xray()));

        source_calibration_fg = 1;
        ui->pushButton_2->setText("Stop");

        // установка времени экспозиции камеры
        AccumulationTime = ui->Exposure->text().toInt();
        reciever->SetAccumulationTime(AccumulationTime);

        // включение источника РИ
        rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
    }
    else
    {
        // обнуление используемых глобальных переменных
        source_calibration_fg = 0;
        source_calibration_step = 0;
        central_point[0] = 0;
        central_point[1] = 0;
        difference_between_centals_point = 0;

        finish_calibration();
    }
}

void MainWindow::onCalibrationGetData(ushort *tdata)
{
    rap->off();
    qDebug() << "OnCalibrationGetData";
    ushort * dData;
    dData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
    memcpy(dData, tdata, IMAGE_WIDTH * IMAGE_HEIGHT * 2);

    frame->setRAWImage(dData);

    source_calibration_step ++;

    int j = 1900;
    int pixel = dData[j];
    int pixel_old = 0;
    int left_limit = 0;
    int right_limit = 0;
    int step_flag_1 = 0;
    //bool step_flag_2 = 0;

    for(int i = 0; i < IMAGE_HEIGHT - 1; i++)
    {
        pixel_old = pixel;
        pixel = dData[i * IMAGE_WIDTH + j];

//        if((pixel_old - pixel) > THRESHOLD)
//        {
//                qDebug() << "right" << pixel_old - pixel << "i" << i;
//        }

//        if((pixel - pixel_old) > THRESHOLD)
//        {
//                qDebug() << "left" << pixel - pixel_old << "i" << i;
//        }

        // Определение левой границы
        if(((pixel_old - pixel) > THRESHOLD) && (step_flag_1 == 0))
        {
             right_limit = i;
             step_flag_1 = 1;
             qDebug() << "right_limit" << right_limit;
        }

        // Определение правой границы
        if(((pixel - pixel_old) > THRESHOLD) && (step_flag_1 == 1))
        {
             left_limit = i;
             step_flag_1 = 2;
             qDebug() << "left_limit" << left_limit;
        }

        // переделать
        // Определение центра изображения
        if((step_flag_1 == 2)/*&&(source_calibration_step == 2)*/)
        {
            step_flag_1 = 0;

            switch (source_calibration_step)
            {
            case 1:
                central_point[0] = -1*(right_limit - left_limit) / 2;
                break;
            case 2:
                central_point[1] = -1*(right_limit - left_limit) / 2;
                difference_between_centals_point = central_point[1] - central_point[0];
                qDebug() << QString("step of calibration = %1").arg(source_calibration_step) << difference_between_centals_point;
                break;
            default:
                break;
            }

        }
    }
    delete[] dData;

    // Проверка окончания работы калибровки
    if(abs(difference_between_centals_point) < DIFFERENCE_LIMIT && (central_point[1] != 0) && (source_calibration_step == 2))
    {
        qDebug () << "finish_calibration";
        emit finish();
        return;
    }

    // перемещение источника
    int step;
    Axes_Mask axes;
    axes = stepmotor_2->reset_axes_mask();
    axes.a4 = 1;
    step = 10000;
    if(source_calibration_step == 2) step *= -1;
    stepmotor_2->go_to_for_calb(step,axes);

    // перемещение источника в сторону (вправо)
    if (difference_between_centals_point < 0)
    {
        axes = stepmotor_2->reset_axes_mask();
        axes.a1 = 1;
        stepmotor_2->go_to_for_calb(-1*STEP_SIZE,axes);
        qDebug() << "move to right" << difference_between_centals_point << central_point[1] << central_point[0];
    }

    // перемещение источника в сторону (влево)
    if (difference_between_centals_point > 0)
    {
        axes = stepmotor_2->reset_axes_mask();
        axes.a1 = 1;
        stepmotor_2->go_to_for_calb(STEP_SIZE,axes);
        qDebug() << "move to left" << difference_between_centals_point << central_point[1] << central_point[0];
    }

    if(source_calibration_step == 2)
    {
        source_calibration_fg = 0;
        source_calibration_step = 0;
        central_point[0] = 0;
        central_point[1] = 0;
        difference_between_centals_point = 0;
    }
}

// new Mode
void MainWindow::on_pushButton_5_clicked()
{
    if(!NewMode_fg)
    {
        // установление соединений для нового режима
        connect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
        connect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onNewModeGetData(ushort *)));
        connect(stepmotor_1,SIGNAL(continue_scan()),this,SLOT(NewMode_continue()));
        connect(stepmotor_2,SIGNAL(continue_scan()),this,SLOT(NewMode_continue()));
        connect(this,SIGNAL(finish()),this,SLOT(NewMode_finish()));
        connect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
        connect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));

        // установка времени экспозиции камеры
        AccumulationTime = ui->Exposure->text().toInt();
        reciever->SetAccumulationTime(AccumulationTime);

        NewMode_fg = 0;
        ui->pushButton_5->setText("Stop");

        NewMode_range = ui->NewMode_Range->text().toUInt();
        NewMode_step = ui->NewMode_Step->text().toInt();

        NewMode_size_of_step = NewMode_range / NewMode_step;


        int start_point = 0;
        Axes_Mask axes;
        axes = stepmotor_2->reset_axes_mask();
        axes.a1 = 1;

        start_point = NewMode_range / 2;

        if(ui->radioButton_2->isChecked())
        {
            qDebug() << "Right to Left";
            start_point *= -1;

        }

        if(ui->radioButton->isChecked())
        {
            qDebug() << "Left to Right";
            NewMode_size_of_step *= -1;
        }

        stepmotor_1->go_to_for_calb(start_point,axes);
        stepmotor_2->go_to_for_calb(start_point,axes);
        // калибровка, исходные точки TODO
        //NewMode_get_start_point();
    }
    else
    {
        // обнуление используемых глобальных переменных
        NewMode_fg = 0;
        NewMode_range = 0;
        NewMode_step = 0;
        NewMode_size_of_step = 0;
        NewMode_current_step = 0;
        ready = 0;
        ui->pushButton_5->setText("NewMode");

        NewMode_finish();
    }
}

// калибровка, исходные точки
void MainWindow::NewMode_get_start_point()
{
    //TODO
}

void MainWindow::NewMode_start_Xray()
{
    // включение источника РИ
   // rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
}


// основная логика
void MainWindow::onNewModeGetData(ushort *tdata)
{
    rap->off();
    NewMode_current_step++;
    qDebug() << "onNewModeGetData";
    ushort * dData;
    dData = new ushort[IMAGE_WIDTH * IMAGE_HEIGHT];
    memcpy(dData, tdata, IMAGE_WIDTH * IMAGE_HEIGHT * 2);

    frame->setRAWImage(dData);

    // сохранение изображений в tiff-формате

    QString NameForSaveImage;
    NameForSaveImage = service_functions::RenameOfImages(NewMode_current_step - 1);
    QString file = FileDirectory + NameForSaveImage;
    tiff->WriteTIFF(file.toStdString(), dData, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 1., 16);

    // проверка завершения
    if (NewMode_current_step > NewMode_step)
    {
        NewMode_finish();
        return;
    }

    // запуск перемещений
    Axes_Mask axes;
    axes = stepmotor_2->reset_axes_mask();
    axes.a1 = 1;

    stepmotor_1->go_to_for_calb(NewMode_size_of_step,axes);
    stepmotor_2->go_to_for_calb(NewMode_size_of_step,axes);
}

// завершение перемещения источника и приемника
void MainWindow::NewMode_continue()
{
    ready++;
    if(ready == 2)
    {
        ready = 0;
        rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
//        if(!rap->xrayStatus) // ?? если рентген выключен
//        {
//            NewMode_start_Xray();
//        }
//        else
//        {
//            reciever->AcquireImage();
//        }
    }
}

void MainWindow::NewMode_finish()
{
    if(rap->xrayStatus)rap->off();

    // разрыв соединений для нового режима
    disconnect(rap, SIGNAL(xrayFound()), reciever, SLOT(AcquireImage()));
    disconnect(reciever, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onNewModeGetData(ushort *)));
    disconnect(stepmotor_1,SIGNAL(continue_scan()),this,SLOT(NewMode_continue()));
    disconnect(stepmotor_2,SIGNAL(continue_scan()),this,SLOT(NewMode_continue()));
    disconnect(this,SIGNAL(finish()),this,SLOT(NewMode_finish()));
    disconnect(rap,SIGNAL(changeI(uint)),this,SLOT(onChangeI(uint)));
    disconnect(rap,SIGNAL(changeU(uint)),this,SLOT(onChangeU(uint)));
}
