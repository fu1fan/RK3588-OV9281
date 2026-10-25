#include "videocapture.h"
#include <linux/v4l2-subdev.h>
#include <linux/v4l2-common.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <QImage>
#include <QDebug>
#include <QDate>
#include <unistd.h>
#include <stdlib.h>

#include "sunxi_camera_v2.h"

#define CLEAR(x) (memset(&(x), 0, sizeof(x)))
#define ALIGN(x,a) (((x) + (a-1)) & ~(a-1))

VideoCapture::VideoCapture(QObject *parent, char *devname,
                           int _width,
                           int _height): QObject(parent)
{
    m_stop = 0;
	take = 0;
    this->width = _width;
    this->height = _height;
    fd = 0;
    strcpy(this->devname, devname);
    buffers = reinterpret_cast<struct buffer *>(::calloc(BUFINFOR_NUM, sizeof(struct buffer)));
}

VideoCapture::~VideoCapture()
{
}

void VideoCapture::SetImg(QLabel *Image)
{
    img = Image;
}

int VideoCapture::Init()
{
	int ret=0,i=0,count=0;

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	struct v4l2_ext_control ctrls[4];
	struct v4l2_ext_controls ext_ctrls;
	struct v4l2_control control;
	struct v4l2_input inp;
	struct v4l2_streamparm parms;
	struct v4l2_requestbuffers req;
	struct v4l2_exportbuffer exp;
	struct v4l2_capability cap;

	fd = ::open(devname, O_RDWR | O_NONBLOCK, 0);
	if (fd < 0) {
		qDebug() << "Open " << devname << "dev fail " <<strerror(errno);
		::exit(1);
	}
	qDebug() << "devname:" << devname << "Video fd: " << fd;

	CLEAR(cap);
	::ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (::strstr(reinterpret_cast<const char *>(cap.driver), "vin") != NULL) {
		csi_type = 1;	
	} else {
		csi_type = 0;
	}

	if (csi_type) {
		inp.index = 0; 
		if (-1 == ioctl(fd, VIDIOC_S_INPUT, &inp)) {
			::exit(1);

		}

		CLEAR(parms);
		parms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		parms.parm.capture.timeperframe.numerator = 1;
		parms.parm.capture.timeperframe.denominator = 30;
		parms.parm.capture.capturemode = V4L2_MODE_VIDEO;
		/* parms.parm.capture.capturemode = V4L2_MODE_IMAGE; */
		/*when different video have the same sensor source, 1:use sensor current win, 0:find the nearest win*/
		parms.parm.capture.reserved[0] = 0;
		parms.parm.capture.reserved[1] = 0; /*2:command, 1: wdr, 0: normal*/

		if (-1 == ioctl(fd, VIDIOC_S_PARM, &parms)) {
			qDebug() << "VIDIOC_S_PARM" << strerror(errno);
			::exit(1);
		}
	}    


	struct v4l2_format fmt;
	CLEAR(fmt);

	if (csi_type) {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		fmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV21;
		fmt.fmt.pix_mp.field = V4L2_FIELD_NONE;
		fmt.fmt.pix_mp.width = this->width;
		fmt.fmt.pix_mp.height = this->height;
	} else {
		fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		fmt.fmt.pix.width = this->width;
		fmt.fmt.pix.height = this->height;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	}
	if (-1 == ioctl(fd, VIDIOC_S_FMT, &fmt)) {
		qDebug() << "VIDIOC_S_FMT" << strerror(errno);
		::exit(1);
	}

	if (-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)) {
		qDebug() << "VIDIOC_G_FMT" << strerror(errno);
		::exit(1);
	} else {
		if (csi_type) {
			nplanes = fmt.fmt.pix_mp.num_planes;
			qDebug() << "resolution got from sensor"
				<< fmt.fmt.pix_mp.width << "*" <<  fmt.fmt.pix_mp.height
				<< " num_planes = " << fmt.fmt.pix_mp.num_planes;
		}
	}

	CLEAR(req);
	req.count = BUFINFOR_NUM;
	if (csi_type) {
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	} else {
		req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	}
	req.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req)) {
		qDebug() << "VIDIOC_REQBUFS" << strerror(errno);
		::exit(1);
	}

	for (int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);
		if (csi_type) {	
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = n_buffers;
			buf.length = nplanes;
			buf.m.planes =
				(struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));
			if (buf.m.planes == NULL) {
				qDebug() << "buf.m.planes calloc" << strerror(errno);
				::exit(1);
			}
		} else {
			buf.index = n_buffers;
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
		}

		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			qDebug() << "VIDIOC_QUERYBUF" << strerror(errno);
			if (csi_type) 
				free(buf.m.planes);
			::exit(1);
		}

		if (csi_type) {
			for (i = 0; i < nplanes; i++) {
				buffers[n_buffers].length[i] = buf.m.planes[i].length;
				buffers[n_buffers].start[i] =
					mmap(NULL,/* start anywhere */
							buf.m.planes[i].length,
							PROT_READ | PROT_WRITE,/* required */
							MAP_SHARED, /* recommended */
							fd, buf.m.planes[i].m.mem_offset);

				if (buffers[n_buffers].start[i] == MAP_FAILED) {
					qDebug() << "mmap failed" << strerror(errno);
					free(buf.m.planes);
					::exit(1);
				}
			}
			free(buf.m.planes);
		} else {
			buf.index = n_buffers;
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			ret = ::ioctl(fd, VIDIOC_QUERYBUF, &buf);
			if(ret < 0){
				perror("VIDIOC_REQBUFS");
				::exit(1);
			}
			buffers[buf.index].length[0] = buf.length;
			buffers[buf.index].start[0] = ::mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, 
					fd, buf.m.offset);
			if(!(buffers[buf.index].start[0])){
				qDebug() << "mmap " << strerror(errno);
				::exit(1);
			}
		}
	}

	for (i = 0; i < BUFINFOR_NUM; ++i) {
		struct v4l2_buffer buf;
		CLEAR(buf);

		if (csi_type) {
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;
			buf.length = nplanes;
			buf.m.planes =
				(struct v4l2_plane *)calloc(nplanes,
						sizeof(struct v4l2_plane));

			if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
				qDebug() << "VIDIOC_QBUF" << strerror(errno);
				free(buf.m.planes);
				::exit(1);
			}
			free(buf.m.planes);
		} else {
			buf.index = i;
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			ret = ::ioctl(fd, VIDIOC_QBUF, &buf);
			if(ret < 0){
				qDebug() << "VIDIOC_QBUF" << strerror(errno);
				::exit(1);
			}
		}
	}

	if (!csi_type) {
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	}
	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type)) {
		qDebug() << "VIDIOC_STREAMON" << strerror(errno);
		::exit(1);
	}

	qDebug() << "Camera Init OK";
	return 0;
}

static int convert_yuv_to_rgb_pixel(int y, int u, int v)
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

static int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
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

void YUV420P_to_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height)
{
	const int nv_start = width * height ;
	int  index = 0, rgb_index = 0;
	uint8_t y, u, v;
	int r, g, b, nv_index = 0,i, j;

	for(i = 0; i < height; i++){
		for(j = 0; j < width; j ++){
			//nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
			nv_index = i / 2  * width + j - j % 2;

			y = yuyv[rgb_index];
			u = yuyv[nv_start + nv_index ];
			v = yuyv[nv_start + nv_index + 1];

			r = y + (140 * (v-128))/100;  //r
			g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
			b = y + (177 * (u-128))/100; //b

			if(r > 255)   r = 255;
			if(g > 255)   g = 255;
			if(b > 255)   b = 255;
			if(r < 0)     r = 0;
			if(g < 0)     g = 0;
			if(b < 0)     b = 0;

			index = rgb_index % width + (height - i - 1) * width;
			//rgb[index * 3+0] = b;
			//rgb[index * 3+1] = g;
			//rgb[index * 3+2] = r;

			//颠倒图像
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 1] = b;
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 2] = g;
			//rgb[height * width * 3 - i * width * 3 - 3 * j - 3] = r;

			//正面图像
			rgb[i * width * 3 + 3 * j + 0] = b;
			rgb[i * width * 3 + 3 * j + 1] = g;
			rgb[i * width * 3 + 3 * j + 2] = r;

			rgb_index++;
		}
	}
	
	return ;
}


int VideoCapture::LoopCapture()
{
	int ret;
	uchar *rgb=new uchar [width * height *3];
	char *align_buffer;

	struct v4l2_buffer buf;

	fd_set fds;
	struct timeval tv;
	int r;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	tv.tv_sec = 2; /* Timeout. */
	tv.tv_usec = 0;
	if (select(fd + 1, &fds, NULL, NULL, &tv) <= 0)
	{
		qDebug() << "timeout";
		return -1;
	}

	CLEAR(buf);

	if (csi_type)
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	else
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	buf.memory = V4L2_MEMORY_MMAP;
	if (csi_type) {
		buf.length = nplanes;
		buf.m.planes =
			(struct v4l2_plane *)calloc(nplanes, sizeof(struct v4l2_plane));
	}
	if (-1 == ioctl(fd, VIDIOC_DQBUF, &buf)) {
		if (csi_type)
			free(buf.m.planes);
		qDebug() << "VIDIOC_DQBUF " << strerror(errno);
		return -1;
	}
	if (csi_type)  {
		switch (nplanes) {
			case 1:
				// convert_yuv_to_rgb_buffer((unsigned char *)buffers[buf.index].start[0], rgb,\
				this->width, this->height);
				align_buffer = reinterpret_cast<char *>(::malloc(this->width * this->height * 1.5));

				if (ALIGN(this->height, 16) != this->height) {
					memcpy(align_buffer, (char *)buffers[buf.index].start[0], this->width * this->height);
					memcpy(align_buffer + this->width * this->height, 
							((char *)buffers[buf.index].start[0]) + 
							ALIGN(this->width, 16) * ALIGN(this->height, 16),
							this->width  * this->height / 2);

					YUV420P_to_RGB24(reinterpret_cast<unsigned char *>(align_buffer),
							rgb, this->width, this->height);
				} else {

					YUV420P_to_RGB24((unsigned char *)buffers[buf.index].start[0],
							rgb, this->width, this->height);
				}                                

				free(align_buffer);
				break;
		}
	} else {
		convert_yuv_to_rgb_buffer((unsigned char *)buffers[buf.index].start[0],rgb, this->width, this->height);
	}

	if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)) {
		qDebug() << "VIDIOC_QBUF " << strerror(errno);
		free(buf.m.planes);
		return -1;
	}

	if(csi_type)
		free(buf.m.planes);

	QImage *mage = new QImage(rgb, width,
			height, QImage::Format_RGB888);
	QImage resultimg = mage->scaled(img->size());


    QMatrix matrix;
    matrix.rotate(180);
    resultimg = resultimg.transformed (matrix);



	img->setPixmap(QPixmap::fromImage(resultimg));

	if (take == 1)
	{
		mage->save(tr("%1.jpg").arg("/root/IMG" + \
					QDate::currentDate().toString("yyyyMMdd") + QTime::currentTime().toString("hhmmss")),"JPG");

		take = 0;
	}

	delete mage;
	delete [] rgb;

	return 0;
}
