#ifndef SERVICE_FUNCTIONS_H
#define SERVICE_FUNCTIONS_H

#include <QObject>
#include <QDebug>
#include "definitions.h"

class service_functions : public QObject
{
    Q_OBJECT
public:
    explicit service_functions(QObject *parent = 0);

signals:

public slots:
    QString RenameOfImages(ushort Count);
};

#endif // SERVICE_FUNCTIONS_H
