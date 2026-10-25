#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QTime>
#include <QTimer>

#include "tinyalsasoundlib.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

class Widget : public QWidget
{
    Q_OBJECT

public:
    struct wav_header {
        uint32_t riff_id;
        uint32_t riff_sz;
        uint32_t riff_fmt;
        uint32_t fmt_id;
        uint32_t fmt_sz;
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t sample_rate;
        uint32_t byte_rate;
        uint16_t block_align;
        uint16_t bits_per_sample;
        uint32_t data_id;
        uint32_t data_sz;
    };
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();

    void on_startBtn_clicked();

    void on_stopBtn_clicked();

private:
    void record_init();

    unsigned int capture_sample();

    unsigned int capture_size;

    FILE *file;
    struct wav_header header;
    unsigned int card = 0;
    unsigned int device = 0;
    unsigned int ahub_card = 0;
    unsigned int ahub_device = 0;
    unsigned int channels = 2;
    unsigned int seconds = 0;
    unsigned int rate = 44100;
    unsigned int bits = 16;
    unsigned int frames;
    unsigned int period_size = 1024;
    unsigned int period_count = 4;
    enum pcm_format format;

    bool capturing;

    QDateTime startTime;
    QTimer timer;
    Ui::Widget *ui;
};
#endif // WIDGET_H
