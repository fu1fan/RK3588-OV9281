#include "camera.h"
#include "ui_camera.h"
#include <QDate>
#include <QTime>

extern "C"{
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <jpeglib.h>
#include <linux/videodev2.h>
}

void yuv422to420p(char *yuv422buf, char *yuv420pbuf,
                int width, int height)
{
        char *src, *dest, *dest2;
        int i, j;

        src = yuv422buf;
        dest = yuv420pbuf;
        for (i = 0; i < width * height * 2; i++) {
                if (i % 2 != 0) {
                        continue;
                }
                *dest++ = *(src + i);
        }

        src = yuv422buf;
        dest = yuv420pbuf + width * height;
        dest2 = dest + (width * height) / 4;
        for (i = 0; i < height; i += 2) {
                for (j = 1; j < width * 2; j += 4) {
                        *dest++ = *(src + i * width * 2 + j);
                        *dest2++ = *(src + i * width * 2 + j + 2);
                }
        }
}

int convert_yuv_to_rgb_pixel(int y, int u, int v)
{
        unsigned int pixel32 = 0;
        unsigned char *pixel = (unsigned char *)&pixel32;
        int r, g, b;
        r = y + (1.370705 * (v-128));
        g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
        b = y + (1.732446 * (u-128));
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;
        if(r < 0) r = 0;
        if(g < 0) g = 0;
        if(b < 0) b = 0;
        pixel[0] = r ;
        pixel[1] = g ;
        pixel[2] = b ;
        return pixel32;
}


int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
         unsigned int in, out = 0;
         unsigned int pixel_16;
         unsigned char pixel_24[3];
         unsigned int pixel32;
         int y0, u, y1, v;

         for(in = 0; in < width * height * 2; in += 4)
         {
                 pixel_16 =
                                 yuv[in + 3] << 24 |
                                 yuv[in + 2] << 16 |
                                 yuv[in + 1] <<  8 |
                                 yuv[in + 0];
                 y0 = (pixel_16 & 0x000000ff);
                 u  = (pixel_16 & 0x0000ff00) >>  8;
                 y1 = (pixel_16 & 0x00ff0000) >> 16;
                 v  = (pixel_16 & 0xff000000) >> 24;
                 pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
                 pixel_24[0] = (pixel32 & 0x000000ff);
                 pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
                 pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
                 rgb[out++] = pixel_24[0];
                 rgb[out++] = pixel_24[1];
                 rgb[out++] = pixel_24[2];
                 pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
                 pixel_24[0] = (pixel32 & 0x000000ff);
                 pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
                 pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
                 rgb[out++] = pixel_24[0];
                 rgb[out++] = pixel_24[1];
                 rgb[out++] = pixel_24[2];
         }
         return 0;
}

int camera::camera_init()
{
	int ret=0,i=0,count=0;
	struct v4l2_capability cap;
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_format format;
	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buf;

	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ::ioctl(fd, VIDIOC_G_FMT, &format);
	if(ret < 0){
		perror("VIDIOC_G_FMT");
		exit(1);
	}

	printf("width:%d\n", format.fmt.pix.width);
	printf("height:%d\n", format.fmt.pix.height);
	printf("pixelformat:%x\n", format.fmt.pix.pixelformat);
	printf("field:%x\n", format.fmt.pix.field);
	printf("bytesperline:%d\n", format.fmt.pix.bytesperline);
	printf("sizeimage:%d\n", format.fmt.pix.sizeimage);
	printf("colorspace:%d\n", format.fmt.pix.colorspace);
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = 640;
	format.fmt.pix.height = 480;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	ret = ::ioctl(fd, VIDIOC_S_FMT, &format);
	if(ret < 0){
		fprintf(stderr, "Not support jepg");
		perror("VIDIOC_S_FMT");
		exit(1);
	}
	reqbuf.count = 3;
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	ret = ::ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if(ret < 0){
		perror("VIDIOC_REQBUFS");
		exit(1);
	}

	bufinf = (struct bufinfor *)calloc(reqbuf.count, sizeof(struct bufinfor));
	if(!bufinf){
		perror("calloc");
		exit(1);
	}
	for(count = 0; count < reqbuf.count; count++){
		buf.index = count;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ::ioctl(fd, VIDIOC_QUERYBUF, &buf);
		if(ret < 0){
			perror("VIDIOC_REQBUFS");
			exit(1);
		}
		bufinf[buf.index].length = buf.length;
		bufinf[buf.index].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if(!(bufinf[buf.index].start)){
			perror("mmap");
			exit(1);
		}
	}
	for(i = 0; i < reqbuf.count; i++){
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ::ioctl(fd, VIDIOC_QBUF, &buf);
		if(ret < 0){
			perror("VIDIOC_QBUF");
			exit(1);
		}
	}
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ::ioctl(fd, VIDIOC_STREAMON, &type);
	if(ret < 0){
		perror("VIDIOC_STREAMON");
		exit(1);
	}

	return 0;
}

camera::camera(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::camera)
{

	char devname[32];
	int i=0;
//	struct v4l2_capability cap;
	ui->setupUi(this);
	while(i < 100)
	{
		sprintf(devname,"/dev/video%d",i++);

		fd = ::open(devname,O_RDWR);
		if(fd < 0)
		{
			continue;
		}

		ui->comboBox->addItem(QWidget::tr(devname));
		::close(fd);
	}
}

camera::~camera()
{
	free(bufinf);

	::close(fd);
	delete ui;
}

void camera::on_pushButton_2_clicked()
{
	take_photo();
}

static bool take = 0;

void camera::show_()
{
	int ret;
	unsigned char *rgb=new unsigned char [640 * 480 *3];
	struct v4l2_buffer buf;

	fd_set readset;
	FD_ZERO(&readset);
	FD_SET(fd, &readset);
	ret = select(fd + 1, &readset, NULL, NULL, NULL);
	if(ret < 0){
		perror("select");
		exit(1);
	}

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd, VIDIOC_DQBUF, &buf);
	if(ret < 0){
		perror("VIDIOC_DQBUF");
		exit(1);
	}

	convert_yuv_to_rgb_buffer((unsigned char *)bufinf[buf.index].start,rgb,640,480);
	ret = ioctl(fd, VIDIOC_QBUF, &buf);
	if(ret < 0){
		perror("VIDIOC_QBUF");
		exit(1);
	}

	QImage *mage = new QImage(rgb,640,480,QImage::Format_RGB888);
	if (take == 1)
	{
    		mage->save(tr("%1.jpg").arg("/root/IMG" + QDate::currentDate().toString("yyyyMMdd") + QTime::currentTime().toString("hhmmss")),"JPG");
		take = 0;
	}
	QImage resultimg=mage->scaled(ui->label->size(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
	ui->label->setPixmap(QPixmap::fromImage(resultimg));
	delete mage;
	delete rgb;

}
void camera::take_photo()
{
	take = 1;
}

void camera::on_comboBox_activated(const QString &arg1)
{
	QString text=ui->comboBox->currentText();
	QByteArray devtext=text.toLatin1();
	char *devname=devtext.data();
    //struct v4l2_capability cap;

	fd = ::open(devname, O_RDWR);

	if(fd < 0)
	{
		perror("open error");
	}
	camera::camera_init();
	QTimer *timer;
	timer=new QTimer();
	timer->setInterval(100);
	connect(timer,SIGNAL(timeout()),this,SLOT(show_()));
	timer->start(100);
	camera_flag=1;
}
