#ifndef CAM_H
#define CAM_H

#include <QObject>

class cam : public QObject
{
    Q_OBJECT
public:
    explicit cam(QObject *parent = 0);

signals:
     void GetDataComplete(ushort*);

public slots:
    virtual void AcquireImage()=0;
    virtual void SetAccumulationTime(int time)=0;
    virtual void Disconnect()=0;
    virtual QString RenameOfImagesTiff()=0;
    virtual QString RenameOfImages()=0;

private slots:
    virtual void WaitForExecution()=0;
};

#endif // CAM_H
