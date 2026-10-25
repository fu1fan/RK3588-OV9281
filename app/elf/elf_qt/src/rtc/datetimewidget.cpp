#include "datetimewidget.h"
#include "ui_datetimewidget.h"
#include <QTimer>
#include <QDebug>
#include "terminal.h"
#include <QTimeZone>
#include "terminal.h"
#include <QDateTime>
#include "messagebox.h"
#include <QLocale>
#include <QTimeZone>
#include <QThread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <QDateTime>
#include <QDebug>
#include "settings.h"

using namespace fl_common;
DateTimeWidget::DateTimeWidget( QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DateTimeWidget),
    m_checker(Q_NULLPTR)
{
    ui->setupUi(this);

    ui->zoneLbl->setText(QTimeZone::systemTimeZoneId());

    auto addDateTime= [=]( QComboBox *pComBox, int start, int end, const QString &unit=""){
        for(int n=start; n<=end; n++){
            QString s = QString::number(n);
            if(n<10){
                s ="0" +s;
            }
            pComBox->addItem(tr((s +" " + unit).toLocal8Bit().data()) , n);
        }
    };
    auto changDay= [=]( ){
        ui->dayComBox->clear();
        QDate d(currentvalue(ui->yearCombox),
                currentvalue(ui->monthComBox),
                1);
        int maxDays = d.daysInMonth();
        addDateTime(ui->dayComBox, 1, maxDays, tr("Day"));
    };
    addDateTime(ui->yearCombox, 2020, 2050, tr("Year"));
    addDateTime(ui->monthComBox, 1, 12, tr("Month"));
    connect(ui->yearCombox, &QComboBox::currentTextChanged, this, [=](){
        changDay();
    });
    connect(ui->monthComBox, &QComboBox::currentTextChanged, this, [=](){
        changDay();
    });
    addDateTime(ui->hourComBox, 0, 24, tr("Houre"));
    addDateTime(ui->minutuComBo, 0, 59, tr("Minute"));

    ui->yearCombox->setCurrentIndex(2);

    auto setEnable= [=](bool checked ){
        ui->yearCombox->setEnabled(checked);
        ui->monthComBox->setEnabled(checked);
        ui->dayComBox->setEnabled(checked);
        ui->hourComBox->setEnabled(checked);
        ui->minutuComBo->setEnabled(checked);
    };

    connect(ui->manualRaidoBtn, &QRadioButton::clicked, this, [=](bool checked){
        setEnable(checked);
    });
    connect(ui->ntpRadioBtn, &QRadioButton::clicked, this, [=](bool checked){
        setEnable(!checked);
    });

    ui->manualRaidoBtn->click();

    m_checker = new DateTimeUpdater(this);
    connect(m_checker, &DateTimeUpdater::updateDateTime, this, [=](const QString &localDateTime, const QString &rtcDateTime)
    {
        ui->systimeDataeLbl->setText(localDateTime);
        ui->rtctimeLbl->setText(rtcDateTime);
    });
    connect(m_checker, &DateTimeUpdater::nptDateTime, this, [=](const QDateTime &date, bool bOk)
    {
        if(bOk)
        {
            MessageBox::showMessage(this, "Set network time OK");
        }else{
            MessageBox::showMessage(this, "Set network time Error");
        }

        ui->applyBtn->setEnabled(true);
    });
}

DateTimeWidget::~DateTimeWidget()
{
    delete ui;
}

int DateTimeWidget::currentvalue(QComboBox *d)
{
    return d->itemData(d->currentIndex()).toInt();
}

void DateTimeWidget::on_applyBtn_clicked()
{
    if(ui->manualRaidoBtn->isChecked())
    {
        QDateTime datetime;
        QDate d(currentvalue(ui->yearCombox),
                currentvalue(ui->monthComBox),
                currentvalue(ui->dayComBox));
        QTime t(currentvalue(ui->hourComBox),
                currentvalue(ui->minutuComBo));
        datetime.setDate(d);
        datetime.setTime(t);
        m_checker->updateRtc(datetime);
    }else{
        if(!m_checker->mayUpdaeteNpt())
        {
            MessageBox::showMessage(this, "Unable to access the network by DNS name.");
            return;
        }

        ui->applyBtn->setEnabled(false);
        m_checker->updateNpt();
    }
}

DateTimeUpdater::DateTimeUpdater(QObject *parent):QThread(parent),
    m_bCanWriteRtc(false)
{
    QString address =Settings::instance()->getValue("rtc_address", "ntp.aliyun.com");
    ProcessHandler::start(QString("ping %1").arg(address));
    QThread::start();
}

DateTimeUpdater::~DateTimeUpdater()
{
    QThread::quit();
    QThread::wait();
    ProcessHandler::stop();
}

void DateTimeUpdater::processMessage(const QString &info)
{
    if(m_info.size() > 300)
        m_info.clear();
    m_info +=info;
}

bool DateTimeUpdater::mayUpdaeteNpt()
{
    bool bOk =m_info.contains("ttl=");
    return bOk;
}

void DateTimeUpdater::run()
{
    QTimer timer;
    connect(&timer, &QTimer::timeout, [=](){
        QString result = Terminal::execCmd("hwclock -r").trimmed();
        foreach (QChar c, result) {
            if(!c.isNumber()){
                m_bCanWriteRtc =false;
                result = "Unknown";
                break;
            }
        }
        emit updateDateTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"), result);
        m_bCanWriteRtc = true;
    });
    timer.start(1000);

    auto setSysDateTime = [=](const QDateTime &date){
        QString cmd =  QString("date -s %1  %2").
                arg("\"" + date.addSecs(1).toString("yyyy-MM-dd hh:mm:ss") + "\"").
                arg("\"+%Y-%m-%d %H:%M:%S\"");
        const char *buf=cmd.toLocal8Bit().constData();
        system(buf);
    };

    connect(this, &DateTimeUpdater::updateNpt, this, [=](){
        long long mSeconds =NTPGetTimeBeijingMSeconds() ;
        QDateTime date= QDateTime::fromMSecsSinceEpoch(mSeconds);
        setSysDateTime(date);
        emit nptDateTime(date, mSeconds >0);
    });

    connect(this, &DateTimeUpdater::updateRtc, this, [=](const QDateTime &date){
        setSysDateTime(date);
        if(m_bCanWriteRtc)
            system("hwclock -w");
    });

    exec();
}

//NTP 标度与 Unix 标度只相差一个常数 2208988800，这是 1900/1/1 （NTP 标度起点）与 1970/1/1 （Unix 标度起点）之间的秒数差别
long long DateTimeUpdater::NTPGetTimeBeijingMSeconds()
{
#define NTP_TIMESTAMP_DELTA 2208988800
    int sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.
    int portno = 123; // NTP UDP port number.
    static QString address =Settings::instance()->getValue("rtc_address", "ntp.aliyun.com");

    char host_name[60];
    memset(host_name, 0, 60);
    strcpy(host_name, address.toLocal8Bit().data());

    typedef struct
    {
        uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
        // li.   Two bits.   Leap indicator.
        // vn.   Three bits. Version number of the protocol.
        // mode. Three bits. Client will pick mode 3 for client.

        uint8_t stratum;         // Eight bits. Stratum level of the local clock.
        uint8_t poll;            // Eight bits. Maximum interval between successive messages.
        uint8_t precision;       // Eight bits. Precision of the local clock.

        uint32_t rootDelay;      // 32 bits. Total round trip delay time.
        uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
        uint32_t refId;          // 32 bits. Reference clock identifier.

        uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
        uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

        uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
        uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

        uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
        uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

        uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
        uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

    } ntp_packet;              // Total: 384 bits or 48 bytes.

    // Create and zero out the packet. All 48 bytes worth.

    ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    memset( &packet, 0, sizeof( ntp_packet ) );

    // Set the first byte’s bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.

    *( ( char * ) &packet + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // Create a UDP socket.

    if ( sockfd < 0 )
    {
        printf("ERROR opening socket\n");
        return 0;
    }

    server = gethostbyname(host_name); // Convert URL to IP.

    if ( server == NULL )
    {
        printf("ERROR, no such host\n");
        return 0;
    }

    // Zero out the server address structure.

    bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );

    serv_addr.sin_family = AF_INET;

    // Copy the server’s IP address to the server address structure.

    bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

    serv_addr.sin_port = htons( portno );

    // Call up the server using its IP address and port number.

    if ( ::connect( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr) ) < 0 )
    {
        printf("ERROR connecting\n");
        return 0;
    }

    n = write( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );

    if ( n < 0 )
    {
        printf("ERROR writing to socket\n");
        return 0;
    }

    long long mSecond = 0;
    // Wait and receive the packet back from the server. If n == -1, it failed.

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    struct timeval tv;
    tv.tv_sec = 1; // 秒数
    tv.tv_usec = 0;      // 微秒数
    // 使用select函数进行等待，并判断是否有数据可读
    int ret = select(sockfd + 1, &fds, NULL, NULL, &tv);
    if(ret <=0)
    {
        printf(" EROOR or timeout");
        return mSecond;
    }

    n = read( sockfd, ( char* ) &packet, sizeof( ntp_packet ) );
    if ( n < 0 )
    {
        printf( "ERROR reading from socket\n");
        return mSecond;
    }

    packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
    packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.
    mSecond =(packet.txTm_s- NTP_TIMESTAMP_DELTA)*1000+ packet.txTm_f*232/1000/1000/1000;
    //qDebug()<<QDateTime::fromMSecsSinceEpoch(mSecond);

    return mSecond;
}

