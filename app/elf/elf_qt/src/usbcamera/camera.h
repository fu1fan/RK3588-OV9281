#ifndef CAMERA_H
#define CAMERA_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QDebug>
#include <QStringList>
#include <QByteArray>
#include <QComboBox>

namespace Ui {
class camera;
}

struct bufinfor{
	void *start;
	unsigned int length;
};

class camera : public QMainWindow
{
    Q_OBJECT

public:
    explicit camera(QWidget *parent = 0);

    ~camera();

private slots:
    int camera_init(void);
    void on_pushButton_2_clicked();
    void show_();
    void take_photo();
    void on_comboBox_activated(const QString &arg1);

private:
    Ui::camera *ui;
	int fd;
	int camera_flag;
	struct bufinfor *bufinf;
};

#endif // CAMERA_H
