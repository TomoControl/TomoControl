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
    if (!file.open(QIODevice::ReadWrite))
    {
        qDebug() << "ошибка";
    }
    QByteArray tmp = file.readAll();
    qDebug() << "service" << tmp;
    file.reset();
    tmp.replace("%20"," ");
    tmp.replace("%28","(");
    tmp.replace("%29",")");
    tmp.replace("%25","%");

    file.write(tmp);

}





