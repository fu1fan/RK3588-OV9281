#include "qplayer.h"
#include <QtWidgets>
#include <QVideoSurfaceFormat>
#include "resourceloader.h"
#include "settings.h"
#include "appwindow.h"
#include <QTime>
using namespace fl_common;

#define MIME_IS_GIF(s)   !(s).compare("image/gif")
#define MIME_IS_IMAGE(s) ((s).startsWith("image/") && !MIME_IS_GIF(s))
#define MIME_IS_VIDEO(s) ((s).startsWith("video/") || MIME_IS_GIF(s))
#define MIME_IS_AUDIO(s) (s).startsWith("audio/")

class VideoWidget : public QVideoWidget
{
public:
    VideoWidget(QPlayer *player ):mPlayer(player){
    }
    virtual void mouseDoubleClickEvent(QMouseEvent *e)override;
private:
    QPlayer *mPlayer=nullptr;
    bool mFullscreen=false;
};

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    QVideoWidget::mouseReleaseEvent(e);
    if(e->button() == Qt::LeftButton && mPlayer)
    {
        mPlayer->setFullScreen(mFullscreen = !mFullscreen);
    }
}

QPlayer::QPlayer()
    : player(nullptr, QMediaPlayer::VideoSurface)
    , list(nullptr)
    , playButton(nullptr)
    , positionSlider(nullptr)
{
    playButton = new QPushButton;
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(playButton, &QAbstractButton::clicked, this, &QPlayer::play);

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 0);
    connect(positionSlider, &QAbstractSlider::sliderMoved, this, &QPlayer::setPosition);

    controlLayout = new QHBoxLayout;
    controlLayout->addWidget(playButton);
    controlLayout->addWidget(positionSlider);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    control = new QWidget();
    control->setLayout(controlLayout);

    videoViewer = new VideoWidget(this);
    videoViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(10, 0,10, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(videoViewer);
    mainLayout->addWidget(control);

    QWidget *widget = new QWidget();
    widget->setLayout(mainLayout);
    setCentralWidget(widget);
    this->layout()->setMargin(0);

    list = new QMediaPlaylist;
    player.setVideoOutput(videoViewer);

    connect(&player, &QMediaPlayer::stateChanged, this, &QPlayer::mediaStateChanged);
    connect(&player, &QMediaPlayer::positionChanged, this, &QPlayer::positionChanged);
    connect(&player, &QMediaPlayer::durationChanged, this, &QPlayer::durationChanged);
    connect(&player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &QPlayer::handleError);

    QString videoPath  =Settings::instance()->getValue("video_path", "/userdata/media/video/");
    QList<QFileInfo> list =ResourceLoader::instacne()->dirRc(videoPath, QDir::Files);
    QStringList fileList;
    for(QFileInfo info: list){
        QString fileName =info.absoluteFilePath();
        if(info.suffix() == "mp4" || info.suffix() == "webm"){
            fileList << fileName;
        }
    }

    if(fileList.size() > 0){
        setPlaylist(fileList);
        player.play();
    }
}

QPlayer::~QPlayer()
{
}

bool QPlayer::isPlayerAvailable() const
{
    return player.isAvailable();
}

void QPlayer::setFullScreen(bool bFullScreen)
{
    if(player.state() == QMediaPlayer::PlayingState)
    {
        videoViewer->setFullScreen(bFullScreen);
        QPalette p =videoViewer->palette();
        QBrush bkColor = this->palette().window();
        bFullScreen ? p.setBrush(QPalette::Background, Qt::black) : p.setBrush(QPalette::Background, bkColor);
        videoViewer->setPalette(p);
    }
}

void QPlayer::next()
{
    list->setCurrentIndex(list->nextIndex());
    play();
}

void QPlayer::setPlaylist(QStringList l)
{
    if(! l.empty()){
        for(int i = 0; i < l.size(); i++){
            QString str = l.at(i);
            QFile f(l.at(i));
            QUrl u(str);

            if(f.exists()){
                list->addMedia(QUrl::fromLocalFile(str));
            }else if(u.isValid()){
                list->addMedia(u);
            }

        }
        list->setCurrentIndex(1);
        player.setPlaylist(list);
    }

    if(list->mediaCount() >= 1){
        list->setPlaybackMode(QMediaPlaylist::Loop);
    } else {
        list->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
    }
}

void QPlayer::load(const QUrl &url)
{
    player.setMedia(url);
    playButton->setEnabled(true);
}

void QPlayer::play()
{
    switch(player.state()) {
    case QMediaPlayer::PlayingState:
        player.pause();
        break;
    default:
        player.play();
        break;
    }
}

void QPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    case QMediaPlayer::StoppedState:
        this->next();
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void QPlayer::positionChanged(qint64 position)
{
    positionSlider->setValue(position);
}

void QPlayer::durationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
}

void QPlayer::setPosition(int position)
{
    if(! player.isSeekable())
        return;
    if(! player.isMuted())
        player.setMuted(true);
    player.setPosition(position);
}

void QPlayer::handleError()
{
    const QString errorString = player.errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(player.error()));
    else
        message += errorString;
    qDebug() << message;

    if(QMessageBox::critical(this, "Qplayer Error", message)){
        exit(0);
    }
}
