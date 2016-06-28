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






