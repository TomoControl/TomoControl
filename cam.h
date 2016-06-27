#ifndef CAM_H
#define CAM_H

#include <QObject>

class cam : public QObject
{
    Q_OBJECT
public:
    explicit cam(QObject *parent = 0);

signals:
    virtual void GetDataComplete(ushort*);

public slots:
    virtual void AcquireImage();
    virtual void SetAccumulationTime(int time);
    virtual void Disconnect();
    virtual QString RenameOfImagesTiff();
    virtual QString RenameOfImages();
};

#endif // CAM_H
