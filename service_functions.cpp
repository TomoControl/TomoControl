#include "service_functions.h"

service_functions::service_functions(QObject *parent) : QObject(parent)
{

}

QString service_functions::RenameOfImages(ushort Count)
{
    Count ++;
    QString FormatOfName;
    FormatOfName = QString("%1").arg(Count);
    while (FormatOfName.length() <= Format_Name_Of_Image)
    {
        FormatOfName = '0' + FormatOfName;
    }
    FormatOfName = "/image_" + FormatOfName + ".raw";
    qDebug() << "ImageCount" << Count;
    return FormatOfName;
}

void service_functions::deletespace(QString adress)
{
    QFile file(adress);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Service::Error";
    }
    QByteArray tmp = file.readAll();
    file.close();

    QFile file_tmp(adress);
    if (file_tmp.open(QIODevice::WriteOnly | QIODevice::Truncate))
    file_tmp.close();

    QFile file_out("s_.log");
    if (!file_out.open(QIODevice::WriteOnly))
    {
        qDebug() << "Service::Error";
    }
    tmp.replace("%20"," ");
    tmp.replace("%28","(");
    tmp.replace("%29",")");
    tmp.replace("%25","%");
    file_out.write(tmp);
}





