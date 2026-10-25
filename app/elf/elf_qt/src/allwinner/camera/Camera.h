#ifndef USBCAMERA_H
#define USBCAMERA_H

#include <QWidget>
#include <QMainWindow>
#include <QThread>
#include "videocapture.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Camera; }
QT_END_NAMESPACE

class Camera : public QWidget
{
    Q_OBJECT

public:
    Camera(QWidget *parent = nullptr);
    ~Camera();

private:
    int threadStatus;
    VideoCapture *capture;
    char DevName[100];
    char DevResolution[100];
private slots:
    void StartWorkThread();
    void StopWorkThread();
    void ApplictionQuit();
    void SaveImge(void);
    void ShowImage(uchar *, int , int );
    void GetVideoDevName(const QString &);
    void GetVideoResolution(const QString &);

private:
    Ui::Camera *ui;
};
#endif // USBCAMERA_H
