#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include <QObject>
#include <QRunnable>
#include <QLabel>
#include <unistd.h>

#define BUFINFOR_NUM  (6)

class bufinfor{
public:
    void *start;
    unsigned int length;
};

struct buffer {
    void *start[3];
    int length[3];
};

class VideoCapture : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit VideoCapture(QObject *parent,
                          char *devname = "/dev/video0",
                          int _width = 1280,
                          int _height = 720);
    int Init(void);
    virtual ~VideoCapture();
    int LoopCapture();

    virtual void run() {
        while (!m_stop) {
            LoopCapture();
        }
        m_stop = 0;
    }

    void stop() {
        if (m_stop == 0) {
            m_stop = 1;
        //    while (m_stop) ::sleep(1);
        }
    }

    void SetImg(QLabel *);
    int take;
    int fd;
    int nplanes;
    struct buffer *buffers;
	int	csi_type;		// 1 csi 0 uvc
private:
    int width;
    int height;

    int m_stop;

    char devname[30];
    QLabel *img;

signals:
    void SetShowImageBuffer(uchar *, int , int);

public slots:
};

#endif // VIDEOCAPTURE_H
