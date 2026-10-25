#include "Camera.h"
#include "ui_Camera.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <QStringList>
#include <QWidget>
#include <QDebug>
#include <QImage>
#include <QMessageBox>
#include <QThreadPool>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>

#include "sunxi_camera_v2.h"
#include "videocapture.h"
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <sys/mman.h>

QLabel *label;
QPushButton *PushStart;
QPushButton *PushStop;
QPushButton *PushPicture;
QPushButton *PushQuit;
QComboBox *comboBox;
QComboBox *resolution;
#define ALIGN(x,a) (((x) + (a-1)) & ~(a-1))

static struct v4l2_capability cap;
Camera::Camera(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Camera)
{
    ui->setupUi(this);
    threadStatus = 0;

    int i=0, ret = 0;
    char devname[100];
    int fd = 0;
    capture = nullptr;
    QGridLayout *gradLayout = new QGridLayout(this);

    comboBox = new QComboBox();
    comboBox->setFixedHeight(20);
    resolution = new QComboBox();
    resolution->setMinimumHeight(20);
    PushStop = new QPushButton();
     PushStop->setMinimumHeight(20);
    PushStop->setText("Stop");
    PushStart = new QPushButton();
    PushStart->setMinimumHeight(20);
    PushStart->setText("Start");
    PushPicture = new QPushButton();
    PushPicture->setMinimumHeight(20);
    PushPicture->setText("picture");

    label = new QLabel;
    label->setText("wait video ....");
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumSize(640, 480);

    gradLayout->addWidget(label, 0, 0, 6, 6);

    QVBoxLayout *verticallayer = new QVBoxLayout();
    verticallayer->setSpacing(20);
    verticallayer->addWidget(comboBox);
    verticallayer->addWidget(resolution);
    verticallayer->addWidget(PushStart);
    verticallayer->addWidget(PushStop);
    verticallayer->addWidget(PushPicture);
    verticallayer->addStretch();
    gradLayout->addLayout(verticallayer, 0, 7, 6, 1);

    comboBox->addItem(QWidget::tr("camera video"));
    resolution->addItem(QWidget::tr("camera resolution"));

    while(i < 100) {
        sprintf(devname,"/dev/video%d",i++);

        fd = ::open(devname,O_RDWR);
        if(fd < 0)
            continue;

        memset(&cap, 0x00, sizeof(cap));
        ret = ::ioctl(fd, VIDIOC_QUERYCAP, &cap);//摄像头主要功能获取
        if ((ret == 0) &&
                ((::strstr(reinterpret_cast<const char *>(cap.driver), "vin") != NULL)
                 || (::strstr(reinterpret_cast<const char *>(cap.driver), "uvc") != NULL)))
        {
            comboBox->addItem(QWidget::tr(devname));
        }
        ::close(fd);
    }

    resolution->addItem(QWidget::tr("640x480"));
    resolution->addItem(QWidget::tr("1280x720"));
    resolution->addItem(QWidget::tr("1920x1080"));
    resolution->addItem((QWidget::tr("2592x1944")));


    connect(PushPicture, SIGNAL(clicked(bool)), this, SLOT(SaveImge()));
    connect(PushStart, SIGNAL(clicked(bool)), this, SLOT(StartWorkThread()));
    connect(PushStop, SIGNAL(clicked(bool)), this, SLOT(StopWorkThread()));
    connect(comboBox,
            SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(GetVideoDevName(const QString &)));

    connect(resolution,
            SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(GetVideoResolution(const QString &)));
}

void Camera::ApplictionQuit()
{
    if (capture) {
        capture->stop();
        QThreadPool::globalInstance()->waitForDone(-1);
    }
}

void Camera::GetVideoDevName(const QString &name)
{
    sprintf(DevName, "%s", name.toStdString().data());
}

void Camera::GetVideoResolution(const QString &name)
{
    sprintf(DevResolution, "%s", name.toStdString().data());
}

Camera::~Camera()
{
    this->ApplictionQuit();
    delete ui;
}

void Camera::SaveImge(void)
{
    if ((capture != nullptr) && (capture->take == 0))
        capture->take = 1;
    //   QMetaObject::connectSlotsByName(this);
}

void Camera::StartWorkThread()
{
    if(!threadStatus) {
        threadStatus = 1;
        const char *res = DevResolution;
        char *devname = DevName;
        if (strcmp("", DevName) == 0) {
            QMessageBox::information(this,
                                     "Title", "no video dev",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes);
            threadStatus = 0;
            return ;
        }

        if (strcmp(res, "") == 0) {
            QMessageBox::information(this,
                                     "Title", "no resolution",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes);
            
            threadStatus = 0;
            return ;
        }

        int videofd = ::open(devname, O_RDWR);
        if (videofd < 0) {
            QMessageBox::information(this,
                                     "Title", "no video device",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes);
            threadStatus = 0;
            return ;
        }

        int w = 0, h = 0;
        int issupport = 0;
        ::sscanf(res, "%dx%d", &w, &h);
        qDebug() << "w:" << w <<"h: " << h;

        memset(&cap, 0x00, sizeof(cap));
        int ret = ::ioctl(videofd, VIDIOC_QUERYCAP, &cap);//摄像头主要功能获取
        if ((ret == 0) &&
                (::strstr(reinterpret_cast<const char *>(cap.driver), "uvc") != NULL)) {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            struct v4l2_fmtdesc fmt_1;
            struct v4l2_frmsizeenum frmsize;
            fmt_1.index = 0;
            fmt_1.type = type;
            while (::ioctl(videofd, VIDIOC_ENUM_FMT, &fmt_1) >= 0) {
                frmsize.pixel_format = fmt_1.pixelformat;
                frmsize.index = 0;
                while (::ioctl(videofd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0){
                    qDebug() << "width: " << frmsize.discrete.width << "height: " << frmsize.discrete.height;
                    if ((w == frmsize.discrete.width)
                            && (h == frmsize.discrete.height)) {
                        issupport = 1;
                        break;
                    }
                    frmsize.index++;
                }

                fmt_1.index++;
            }

            if (!issupport) {
                QMessageBox::information(this,
                                         "Title", "not support resolution",
                                         QMessageBox::Yes | QMessageBox::No,
                                         QMessageBox::Yes);
                ::close(videofd);
                return ;
            }
        }
        ::close(videofd);


        if ((w != 0) && (h != 0)) {
            capture = new VideoCapture(this,
                                       devname, w, h);
        } else {
            QMessageBox::information(this,
                                     "Title", "no resolution",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes);
            return ;
        }

        if (capture->Init() >= 0) {

            //  connect(capture, SIGNAL(SetShowImageData(uchar *, int , int )),\
            //this, SLOT(&Camera::ShowImage(uchar *, int , int )));
            capture->SetImg(label);
            capture->setAutoDelete(true);
            QThreadPool::globalInstance()->setMaxThreadCount(1);
            QThreadPool::globalInstance()->setExpiryTimeout(2);
            QThreadPool::globalInstance()->start(capture);
        }

    }
    //    QMetaObject::connectSlotsByName(this);
}


void Camera::StopWorkThread()
{
    int i = 0;
    int ret;
    if(threadStatus) {
#if 1
        capture->stop();
        QThreadPool::globalInstance()->waitForDone(-1);

        if(capture->csi_type) {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            ret = ::ioctl(capture->fd, VIDIOC_STREAMOFF, &type);
            if(ret < 0) {
                qDebug() << "VIDIOC_STREAMOFF" << strerror(errno);
            }

            for (i = 0; i < BUFINFOR_NUM; i++) {
                for (int j = 0; j < capture->nplanes; j++) {
                    ::munmap(capture->buffers[i].start[j], capture->buffers[i].length[j]);
                }
            }
        } else {
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ret = ::ioctl(capture->fd, VIDIOC_STREAMOFF, &type);
            if(ret < 0) {
                qDebug() << "VIDIOC_STREAMOFF" << strerror(errno);
            }

            for (i = 0; i < BUFINFOR_NUM; i++) {
                ::munmap(capture->buffers[i].start[0], capture->buffers[i].length[0]);
            }
        }

        ::free(capture->buffers);
        ::close(capture->fd);

#endif
        threadStatus = 0;
    }
}

void Camera::ShowImage(uchar *rgb, int width, int height)
{
    Q_UNUSED(rgb)
    Q_UNUSED(width)
    Q_UNUSED(height)
    //    QImage *mage = new QImage(rgb, width,
    //                              height, QImage::Format_RGB888);
    //    QImage resultimg = mage->scaled(ui->label->size());
    //    ui->label->setPixmap(QPixmap::fromImage(resultimg));

    //    delete mage;
}

