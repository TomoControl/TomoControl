﻿#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // инициализация части переменных
    status = 0; // метка состояния сканирования: включено - 1 / выключено - 0
    XrayStatus = 0;
    CountOfShoot = 0;

    // графический интерфейс
    ui->setupUi(this);
    ui->with_rotate->setChecked(true);

    tiff = new tiff_image;

    dialog = new Dialog;

    // создание поля для изображения
    frame = new myFrame;
    ui->frame_3->setLayout(new QHBoxLayout);
    ui->frame_3->layout()->addWidget(frame);

    // создание гистограммы
    graphicsScene = new myGraphicsScene(ui->graphicsView->width(), ui->graphicsView->height());
    ui->graphicsView->setScene(graphicsScene);

    // инициализация драйверов ШД
    stepmotor = new stepmotor_rotate;
    stepmotor_2 = new stepmotor_rotate;
    Source = ("192.168.10.1");
    SourcePort = 1075;
    Destination = ("192.168.10.11");
    DestinationPort = 5000;
    stepmotor->initialization(Source, Destination, SourcePort, DestinationPort);
    SourcePort = 1234;
    Destination = ("192.168.10.10");
    stepmotor_2->initialization(Source, Destination, SourcePort, DestinationPort);

    Timer = new QTimer;
    QObject::connect(Timer , SIGNAL(timeout()) , this, SLOT(myTimer()));
    Timer->start(200);

    // инициализация приемника РИ
    cam = new AlphaCam;

    // инициализация источника РИ
    rap = new RAPEltechMED;
    rap->initialization();

    // установление соединений
    connect(frame, SIGNAL(histCalculated(ushort*)), graphicsScene, SLOT(onHistCalculated(ushort*)));
    connect(graphicsScene, SIGNAL(changeHistOutput(int,int)), frame, SLOT(onChangeHistogrammWidget(int,int)));

}

MainWindow::~MainWindow()
{
    // разрыв соединений
    disconnect(frame, SIGNAL(histCalculated(ushort*)), graphicsScene, SLOT(onHistCalculated(ushort*)));
    disconnect(graphicsScene, SIGNAL(changeHistOutput(int,int)), frame, SLOT(onChangeHistogrammWidget(int,int)));

    // отключение и осовобождение памяти для драйверов ШД
    Timer->stop();
    QObject::disconnect(Timer , SIGNAL(timeout()) , this , SLOT(myTimer()));
    stepmotor->go_emergency();
    stepmotor_2->go_emergency();

    delete stepmotor , stepmotor_2, Timer;

    // отключение и осовобождение памяти для приемника РИ
    cam->Disconnect();
    delete cam;

    // отключение и осовобождение памяти для источника РИ
    rap->ClosePort();
    delete rap;

    delete frame , graphicsScene;
    delete ui;
}

void MainWindow::myTimer()    //действие по таймеру
{
    // отображение текущей координаты ШД
    show_current_position show;
    show = stepmotor->get_current_position();
    ui->pos1->setText(QString::number(show.Position_2));

    // отображение текущего шага сканирования
    ui->current_step->setText(QString::number(CountOfShoot));

    // отображение ошибок
    //show_errors();
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
    connect(dialog,SIGNAL(close_dialog()),this,SLOT(close_dialog()));
    connect(dialog,SIGNAL(move(Axes_Mask,int)),stepmotor,SLOT(manual_movement(Axes_Mask,int)));
    connect(dialog,SIGNAL(stop(Axes_Mask)),stepmotor,SLOT(stop_movement(Axes_Mask)));
    connect(dialog,SIGNAL(move_2(Axes_Mask,int)),stepmotor_2,SLOT(manual_movement(Axes_Mask,int)));
    connect(dialog,SIGNAL(stop_2(Axes_Mask)),stepmotor_2,SLOT(stop_movement(Axes_Mask)));
    dialog->set_icons();
    dialog->show();
}

void MainWindow::make_shoot(uchar U, uchar I, int time)
{
    rap->setU(U);
    rap->setI(I);
    cam->SetAccumulationTime(time);

    connect(rap, SIGNAL(xrayFound()), cam, SLOT(AcquireImage()));
    connect(cam, SIGNAL(GetDataComplete(ushort*)),dialog,SLOT(set_image(ushort*)));
}

// деактивация диалогового окна
void MainWindow::close_dialog()
{
    disconnect(rap, SIGNAL(xrayFound()), cam, SLOT(AcquireImage()));
    disconnect(cam, SIGNAL(GetDataComplete(ushort*)),dialog,SLOT(set_image(ushort*)));
    disconnect(dialog,SIGNAL(close_dialog()),this,SLOT(close_dialog()));
    disconnect(dialog,SIGNAL(move(Axes_Mask,int)),stepmotor,SLOT(manual_movement(Axes_Mask,int)));
    disconnect(dialog,SIGNAL(stop(Axes_Mask)),stepmotor,SLOT(stop_movement(Axes_Mask)));
    disconnect(dialog,SIGNAL(move_2(Axes_Mask,int)),stepmotor_2,SLOT(manual_movement(Axes_Mask,int)));
    disconnect(dialog,SIGNAL(stop_2(Axes_Mask)),stepmotor_2,SLOT(stop_movement(Axes_Mask)));
    dialog->hide();
}


// ******************** Автоматическое управление системой ********************* //

// Запуск автоматического набора проекционных данных
void MainWindow::on_Start_AutoScan_clicked()
{
    if(!status)
    {
        // желаемое количество проекций
        NumberOfImage = ui->NumberOfSteps->text().toInt();
        // индикация процесса выполнения набора проекций
        ui->progressBar->setRange(0,NumberOfImage);

        // расчет длины 1 шага при перемещении объекта
        if ((FULL_TURN % NumberOfImage) == 0)
        {
            SizeOfStep = FULL_TURN / NumberOfImage;
            ui->ErrorLabel->setText("");
        }
        else
        {
            ui->ErrorLabel->setText("Введите корректное число проекций");
            return;
        }

        // установление соединений для автосканирования
        connect(rap, SIGNAL(xrayFound()), cam, SLOT(AcquireImage()));
        connect(cam, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onGetData(ushort *)));
        connect(this, SIGNAL(nextStep(int,int)), stepmotor, SLOT(calculate_go(int,int)));
        connect(stepmotor, SIGNAL(continue_move()), cam, SLOT(AcquireImage()));
        connect(this, SIGNAL(finishAutoScan()), this, SLOT(finish_autoscan()));
        connect(this,SIGNAL(retry_acquire_image()),cam,SLOT(AcquireImage()));

        status = 1;
        CountOfShoot = 0;
        ui->Start_AutoScan->setText("Stop AutoScan");

        // установка времени экспозиции камеры
        AccumulationTime = ui->Exposure->text().toInt();
        cam->SetAccumulationTime(AccumulationTime);
        // включение источника РИ
        rap->on((uchar)ui->U_Auto->text().toShort(), (uchar)ui->I_Auto->text().toShort());
    }
    else
    {
        finish_autoscan();
    }
}

void MainWindow::finish_autoscan()
{
    // отключение соединений, необходимых для автосканирования
    disconnect(rap, SIGNAL(xrayFound()), cam, SLOT(AcquireImage()));
    disconnect(cam, SIGNAL(GetDataComplete(ushort*)), this, SLOT(onGetData(ushort *)));
    disconnect(this, SIGNAL(nextStep(int,int)), stepmotor, SLOT(calculate_go(int,int)));
    disconnect(stepmotor, SIGNAL(continue_move()), cam, SLOT(AcquireImage()));
    disconnect(this, SIGNAL(finishAutoScan()), rap, SLOT(off()));
    disconnect(this,SIGNAL(retry_acquire_image()),cam,SLOT(AcquireImage()));

    // выключение источника, сброс индикации
    rap->off();
    status = 0;
    ui->Start_AutoScan->setText("Start AutoScan");
}

// прием изображения от камеры
void MainWindow::onGetData(ushort * tdata)
{
        qDebug() << "OnGetData";
        ushort * dData;
        dData = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
        memcpy(dData, tdata, IMAGE_WIDTH*IMAGE_HEIGHT*2);

        if (CountOfShoot == 0)
        {
            qDebug() << "mainwindow :: create ini";

            // создание ini-файла с параметрами съемки
            QString Time_start = QTime::currentTime().toString("hh-mm-ss");

            // выбор директории для сохранения
            chooseDirectory(1);

            QSettings *setting = new QSettings ( FileDirectory , QSettings::IniFormat);
            setting->setValue("NumberOfImage" , NumberOfImage);
            setting->setValue("Current" , (uchar)ui->I_Auto->text().toShort());
            setting->setValue("Voltage" , (uchar)ui->U_Auto->text().toShort());
            setting->setValue("StartTime" , Time_start);
            setting->sync();
            chooseDirectory(2);
            settingtxt = new QSettings ( FileDirectory + "txt.ini" , QSettings::IniFormat);
        }

        int avpixel, pixel;
        int needRenew = 0;

        avpixel = dData[0];
        pixel = 0;

        // расчет среднего значения пиксела
        for (int k=0; k<50; k++)
        {
            for (int j=0; j<50; j++)
            {
                pixel = dData[(k*IMAGE_WIDTH)+j];
                avpixel = (avpixel + pixel)/2;
            }
        }
        qDebug() << "avpixel = " << avpixel;

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
                cam->SetAccumulationTime(AccumulationTime);
                needRenew = 1;
            }
            else if (avpixel - avFirstImage < (-1*ui->Compare->text().toInt()))
            {
                AccumulationTime -= ui->TimeCorrect->text().toInt();
                cam->SetAccumulationTime(AccumulationTime);
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

        qDebug() << "Cканирование:: Рентгеновский снимок" << CountOfShoot + 1 << "iz" << NumberOfImage;
        qDebug() << "Cканирование:: Получение снимка завершено";

        if (!needRenew)
        {
            // сохранение среднего значения пиксела на снимке
            QString txt = QString("Average%1").arg(CountOfShoot);
            settingtxt->setValue(txt,avpixel);
            //settingtxt->setValue("avpixel" , avpixel);
            //settingtxt->setValue(txt, AccumulationTime);
            settingtxt->sync();

            // сохранение изображений в raw-формате
            QString NameForSaveImage;
            NameForSaveImage = cam->RenameOfImages();
            QFile file(FileDirectory + NameForSaveImage);
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "Cканирование:: Сохранение изображения:: Ошибка открытия файла";
            }
            file.write((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);
            frame->setRAWImage(dData);
            CountOfShoot++;
        }

        delete[] dData;
        ui->progressBar->setValue(CountOfShoot);

        if (CountOfShoot == NumberOfImage)
        {
            emit finishAutoScan();
            return;
        }
        emit nextStep(SizeOfStep, CountOfShoot);
}

// слот по обработке изменения выбранного снимка в CocmboBox
void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    QString CheckedImage;
    if (index < 9) CheckedImage = QString("/image_000%1.raw").arg(index+1);
    if (index >=9 && index < 99) CheckedImage = QString("/image_00%1.raw").arg(index+1);
    if (index >=99) CheckedImage = QString("/image_0%1.raw").arg(index+1);
    ushort * dData;
    dData = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
    QString Dir = QString(FileDirectory + CheckedImage);
    QFile file(Dir);
    qDebug() << "load" << FileDirectory + CheckedImage ;
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Загрузка изображения::Ошибка открытия файла";
    }
    file.read((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);

    frame->setRAWImage(dData);
    // контрастирование по выставленной гистограмме
    graphicsScene->onHistChanged();

    delete[] dData;

}

// загрузка полученных ранее проекционных данных
void MainWindow::on_load_image_clicked()
{
    ui->comboBox->clear();
    chooseDirectory(4);
    uchar CountOfImage;
    QSettings *setting_stop = new QSettings ( FileDirectory, QSettings::IniFormat );
    CountOfImage = setting_stop->value("NumberOfImage" , 0).toInt();
    chooseDirectory(2);
    for(uint i = 1; i <= CountOfImage; i++)
    { // подгрузка в комбобок при автосканировании
        QString CheckedImage;
        if (i < 9) CheckedImage = QString("image_000%1.raw").arg(i);
        if (i >=9 && i < 99) CheckedImage = QString("image_00%1.raw").arg(i);
        if (i >=99) CheckedImage = QString("image_0%1.raw").arg(i);
        ui->comboBox->addItem(CheckedImage);
    }  
}

//  конвертирование изображения в 8-битное
void MainWindow::on_convert_image_clicked()
{
    if (ui->image_format->currentIndex() == 0) convertToTiff();
    if (ui->image_format->currentIndex() == 1) convertTo8Bit();
}

void MainWindow::convertToTiff()
{
    chooseDirectory(4);
    QString CurrentPicture;
    ushort CountOfImage;
    ushort * dData;
    dData = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
    QSettings *setting_2 = new QSettings (  FileDirectory , QSettings::IniFormat ); // &\? directory of .ini
    CountOfImage = setting_2->value("NumberOfImage" , 0).toInt();
    qDebug() << "Конвертирование:: Число изображений для конвертации:" << CountOfImage;
    chooseDirectory(2);
    int min = 62341;
    int max = 0;
    for (uint i = 1; i <= CountOfImage; i++)
    {
        if (i < 10) CurrentPicture = QString("/image_000%1.raw").arg(i);
        if (i >=10 && i < 100) CurrentPicture = QString("/image_00%1.raw").arg(i);
        if (i >=100) CurrentPicture = QString("/image_0%1.raw").arg(i);
        QFile file(FileDirectory + CurrentPicture);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Конвертирование::Ошибка открытия файла";
        }
        file.read((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);

        QString NameForSave;
        if (i < 11) NameForSave = QString("/s_000%1.tif").arg(i-1);
        if (i >=11 && i < 101) NameForSave = QString("/s_00%1.tif").arg(i-1);
        if (i >=101) NameForSave = QString("/s_0%1.tif").arg(i-1);


        if (i == 1)
        {
            // Расчет гистограммы первого файла
            int pixel = 0;
            uint * hist = new uint[65535];
            for (int k = 0; k < 65535; k++)
            {
                hist[k] = 0;
            }

            for (int k=0;k<IMAGE_HEIGHT-1;k++)
            {
                for (int j=0; j<IMAGE_WIDTH-1; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j];
                    hist[pixel]++;
                }
            }

            for (int k = 0; k < 65535; k++)
            {
                if (hist[k] > 100)
                {
                    min = k;
                    break;
                }
            }

            for (int k = 65535; k > 0; k--)
            {
                if (hist[k] > 100)
                {
                    max = k;
                    break;
                }
            }

            min -= 1000;
            max += 1000;
            qDebug() << "min" << min << "max" << max;
            pixel = 0;

            // Коррекция первого файла по автоматически вычисленным границам
            for (int k=0;k<IMAGE_HEIGHT-1;k++)
            {
                for (int j=0; j<IMAGE_WIDTH-1; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j];
                    pixel = 65535  * (pixel - min) / (max - min) ;
                    if (pixel>65535) pixel = 65535;
                    if (pixel<0) pixel = 0;
                    dData[(k*IMAGE_WIDTH)+j] = 65535 - pixel;
                }
            }
        }
        else
        {
            int pixel = 0;
            pixel = 0;
            for (int k=0;k<IMAGE_HEIGHT-1;k++)
            {
                for (int j=0; j<IMAGE_WIDTH-1; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j];
                    pixel = 65535 * (pixel - min) / (max - min);
                    if (pixel>65535) pixel = 65535;
                    if (pixel<0) pixel = 0;
                    dData[(k*IMAGE_WIDTH)+j] = 65535 - pixel;
                }
            }
        }

        tiff->WriteTIFF(FileDirectory.toStdString() + NameForSave.toStdString(),
                        dData, IMAGE_WIDTH, IMAGE_HEIGHT, 0, 1., 16);
    }
}

void MainWindow::convertTo8Bit()
{
    chooseDirectory(4);
    QString CurrentPicture;
    ushort CountOfImage;
    ushort * dData;
    int avpixel = 0;
    int avpixel_new = 0;
    dData = new ushort[IMAGE_WIDTH*IMAGE_HEIGHT];
    QSettings *setting_2 = new QSettings (  FileDirectory , QSettings::IniFormat ); // &\? directory of .ini
    CountOfImage = setting_2->value("NumberOfImage" , 0).toInt();
    qDebug() << "Конвертирование convertTo8Bit:: Число изображений для конвертации:" << CountOfImage;
    chooseDirectory(2);
    for (uint i = 1; i <= CountOfImage; i++)
    {
        if (i < 10) CurrentPicture = QString("/image_000%1.raw").arg(i);
        if (i >=10 && i < 100) CurrentPicture = QString("/image_00%1.raw").arg(i);
        if (i >=100) CurrentPicture = QString("/image_0%1.raw").arg(i);
        QFile file(FileDirectory + CurrentPicture);
        if (!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "Конвертирование::Ошибка открытия файла";
        }
        file.read((char*)dData, IMAGE_WIDTH*IMAGE_HEIGHT*2);

        QString NameForSave;
        if (i < 10) NameForSave = QString("/image_000%1.jpg").arg(i);
        if (i >=10 && i < 100) NameForSave = QString("/image_00%1.jpg").arg(i);
        if (i >=100) NameForSave = QString("/image_%01.jpg").arg(i);

        if (i == 1)
        {
            avpixel = dData[0];
            int pixel = 0;
            for (int k=0; k<50; k++)
            {
                for (int j=0; j<50; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j];
                    avpixel = (avpixel + pixel)/2;
                }
            }
            qDebug() << "average" << avpixel;
        }
        else
        {
            avpixel_new = dData[0];
            int pixel = 0;
            for (int k=0; k<100; k++)
            {
                for (int j=0; j<100; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j];
                    avpixel_new = (avpixel_new + pixel)/2;
                }
            }
            qDebug() << "average_new do" << avpixel_new;
            avpixel_new -= avpixel;
            qDebug() << "average_new posle" << avpixel_new;
            pixel = 0;
            for (int k=0;k<IMAGE_HEIGHT-1;k++)
            {
                for (int j=0; j<IMAGE_WIDTH-1; j++)
                {
                    pixel = dData[(k*IMAGE_WIDTH)+j] - avpixel_new;
                    dData[(k*IMAGE_WIDTH)+j] = pixel;
                }
            }
        }

        QImage image(IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB32 );
        int pixel = 0;
        for (int k=0;k<IMAGE_HEIGHT-1;k++)
        {
            for (int j=0; j<IMAGE_WIDTH-1; j++)
            {
                pixel = dData[(k*IMAGE_WIDTH)+j]/ 64;
                if (pixel > 255) pixel = 255;
                if (pixel < 0) pixel = 0;
                image.setPixel(j,k,QColor(pixel,pixel,pixel,255).rgba());
            }
        }
        image.save(FileDirectory + NameForSave);
    }
    delete[] dData;
}

// выбор директории для сохранения/загрузки полученных изображений
void MainWindow::chooseDirectory(uchar stage)
{
    switch (stage)
    {
    case 1: // путь к ini-файлу в автоматическом и ручном режимах
    {
        QString date = QDate::currentDate().toString("yyyy-MM-dd");
        QString time = QTime::currentTime().toString("hh-mm-ss");
        QString auto_single = "AutoScan";
        FileDirectory = QString("%1/%2/%3/ShootingMode.ini" ).arg(auto_single).arg(date).arg(time);
        break;
    }
    case 2:  // путь для сохранения изображений в автоматическом и ручном режимах
    {
        FileDirectory = FileDirectory.remove("ShootingMode.ini");
        FileDirectory = FileDirectory.remove("AutoContrast.ini");
        break;
    }
    case 3:   // путь к ini-файлу в автоматическом и ручном режимах для дозаписи в него данных о съемке
    {
        FileDirectory += "ShootingMode.ini";
        break;
    }
    case 4:  // путь для загрузки полученных ранее изображений
    {
        FileDirectory = QFileDialog::getOpenFileName(0,"Выбор файла", "", "ShootingMode.ini");
        break;
    }
    case 5:  // путь для загрузки полученных ранее изображений
    {
        FileDirectory = QFileDialog::getOpenFileName(0,"Выбор файла", "", "AutoContrast.ini");
        break;
    }
    default:
        break;
    }
}

void MainWindow::on_SaveAutoContrast_clicked()
{
    chooseDirectory(4);
    FileDirectory.remove("ShootingMode.ini");
    FileDirectory += "AutoContrast.ini";
    QSettings *setting = new QSettings ( FileDirectory , QSettings::IniFormat);
    setting->setValue("left_limit" , 270 - graphicsScene->lineLow->rect().width());
    setting->setValue("right_limit" , graphicsScene->lineHigh->scenePos().x());
    setting->setValue("height" , graphicsScene->lineLow->rect().height());
    setting->setValue("width" , graphicsScene->lineLow->rect().width());
    chooseDirectory(2);
}

void MainWindow::on_LoadAutoContrast_clicked()
{
    chooseDirectory(5);
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
    chooseDirectory(2);
}

// проверка корректности ввода желаемого числа проекций
void MainWindow::on_NumberOfSteps_textChanged(const QString &arg1)
{
    if ((ui->NumberOfSteps->text().toInt() == 1 && ui->with_rotate->isChecked()) ||
        (ui->NumberOfSteps->text().toInt() != 1 && !ui->with_rotate->isChecked()) ||
         ui->NumberOfSteps->text().toInt() < 1 )
        ui->Start_AutoScan->setEnabled(false);
    else
        ui->Start_AutoScan->setDisabled(false);
}

//????
void MainWindow::on_with_rotate_stateChanged(int arg1)
{
    if ((ui->NumberOfSteps->text().toInt() == 1 && ui->with_rotate->isChecked()) ||
        (ui->NumberOfSteps->text().toInt() != 1 && !ui->with_rotate->isChecked()) ||
         ui->NumberOfSteps->text().toInt() < 1 )
        ui->Start_AutoScan->setEnabled(false);
    else
        ui->Start_AutoScan->setDisabled(false);
}


string MainWindow::qstr2str(QString x)
{
    std::stringstream ss;
    ss << x.toStdString();
    return ss.str();
}


