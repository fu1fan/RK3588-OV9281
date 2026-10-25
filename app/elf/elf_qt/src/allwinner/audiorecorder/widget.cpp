#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QCoreApplication>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->lcdNumber->setDigitCount(8);

    ui->dirEdit->installEventFilter(this);

    capturing = false;
    record_init();

    connect(&timer, &QTimer::timeout, this, [=](){
        QTime m_time;
        m_time.setHMS(0, 0, 0, 0);
        QString timestr = m_time.addSecs(startTime.secsTo(QDateTime::currentDateTime())).toString("hh:mm:ss");
        ui->lcdNumber->display(timestr);
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::record_init()
{
    card = 3;
    device = 0;
    ahub_card = 1;
    ahub_device = 1;

    channels = 2;
    rate = 44100;
    bits = 16;
    period_size = 1024;
    period_count = 4;
    format = PCM_FORMAT_S16_LE;

    header.riff_id = ID_RIFF;
    header.riff_sz = 0;
    header.riff_fmt = ID_WAVE;
    header.fmt_id = ID_FMT;
    header.fmt_sz = 16;
    header.audio_format = FORMAT_PCM;
    header.num_channels = channels;
    header.sample_rate = rate;

    header.bits_per_sample = pcm_format_to_bits(format);
    header.byte_rate = (header.bits_per_sample / 8) * channels * rate;
    header.block_align = channels * (header.bits_per_sample / 8);
    header.data_id = ID_DATA;
}


unsigned int Widget::capture_sample()
{
    struct pcm_config config;
    struct pcm *pcm;
    struct pcm *ahub_pcm;
    char *buffer;
    unsigned int size;
    unsigned int frames = 0;
    unsigned int bytes_read = 0;

    memset(&config, 0, sizeof(config));
    config.channels = channels;
    config.rate = rate;
    config.period_size = period_size;
    config.period_count = period_count;
    config.format = format;
    config.start_threshold = 0;
    config.stop_threshold = period_count * period_size;
    config.silence_threshold = 0;

    pcm = pcm_open(card, device, PCM_IN, &config);
    if (!pcm || !pcm_is_ready(pcm)) {
        fprintf(stderr, "Unable to open PCM device (%s)\n",
                pcm_get_error(pcm));
        return -1;
    }
    pcm_start(pcm);

    ahub_pcm = pcm_open(ahub_card, ahub_device, PCM_IN, &config);
    if (!ahub_pcm || !pcm_is_ready(ahub_pcm)) {
        fprintf(stderr, "Unable to open AHub device %d (%s)\n",
                ahub_device, pcm_get_error(ahub_pcm));
        pcm_close(pcm);
        return -1;
    }

    size = pcm_frames_to_bytes(ahub_pcm, pcm_get_buffer_size(ahub_pcm));
    buffer = (char *)malloc(size);
    if (!buffer) {
        fprintf(stderr, "Unable to allocate %d bytes\n", size);
        free(buffer);
        pcm_close(ahub_pcm);
        pcm_close(pcm);
        return -1;
    }

    printf("Capturing sample: %u ch, %u hz, %u bit, malloc buffer:%d\n",
            channels, rate, pcm_format_to_bits(format), size);

    while (capturing && !pcm_read(ahub_pcm, buffer, size)) {

        if (fwrite(buffer, 1, size, file) != size) {
            fprintf(stderr,"Error capturing sample\n");
            break;
        }
        bytes_read += size;
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    frames = pcm_bytes_to_frames(ahub_pcm, bytes_read);

    free(buffer);
    pcm_close(ahub_pcm);
    pcm_close(pcm);

    return frames;
}


void Widget::on_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                     "/",
                                                     QFileDialog::ShowDirsOnly
                                                     | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        ui->dirEdit->setText(dir + "/record.wav");
}

void Widget::on_startBtn_clicked()
{
    file = fopen(ui->dirEdit->text().toLocal8Bit().data(), "wb");
    if (!file) {
        qDebug()<<ui->dirEdit->text()<<" open failed";
        return;
    }

    ui->startBtn->setEnabled(false);
    capturing = true;

    /* leave enough room for header */
    fseek(file, sizeof(struct wav_header), SEEK_SET);

    startTime = QDateTime::currentDateTime();
    timer.start(10);

    int frames = capture_sample();

    /* write header now all information is known */
    header.data_sz = frames * header.block_align;
    header.riff_sz = header.data_sz + sizeof(header) - 8;
    fseek(file, 0, SEEK_SET);
    fwrite(&header, sizeof(struct wav_header), 1, file);
    fclose(file);
}

void Widget::on_stopBtn_clicked()
{
    timer.stop();
    capturing = false;
    ui->startBtn->setEnabled(true);
}
