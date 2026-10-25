#include "keyboardwidget.h"
#include "iminputcontext.h"
#include "implatforminputcontext.h"
#include "abstractinputmethod.h"
#include "inputmethodmanager.h"
#include <QListView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QWindow>
#include <QStringListModel>
#include <QScreen>
#include <QtMath>
#include <QTimer>

#define  PageKey Qt::Key_unknown+1
#define  LanguageKey Qt::Key_unknown+2
#define  CloseKey     Qt::Key_unknown+3
#define  NotUsedKey    Qt::Key_unknown+6
#define  PAGEKEYCOUNT 39

class KeyboardWidgetPrivate{
public:
    QWidget *keyboardWidget=nullptr;
    QListView *listView=nullptr;
    QStringListModel model;

    QList<ButtonKey*> buttons;

    QList<int> pageKeys1;
    QList<int> pageKeys2;
    QList<int> pageKeys3;

    static int currentPage;
    bool bBigLetter =false;
    bool bLetterPage =true;

    QRect rect;
    bool bSupport =true;
    void updatPage(int nPage =-1);
    void updateState();
};

void KeyboardWidgetPrivate::updatPage(int nPage)
{
    QList<int> keys;;

    if(nPage == -1)
        nPage = currentPage;

    switch (nPage) {
    case 0:{
        keys = pageKeys1;
        currentPage =0;
    }break;
    case 1: {
        keys = pageKeys2;;
        currentPage =1;
    }break;
    case 2:{
        keys = pageKeys3;
        currentPage =2;
    }
        break;
    default:
        break;
    }

    AbstractInputMethod *pInstance = InputMethodManager::instance()->curentInputMethod();
    for(int i=0; i<keys.size() && i<buttons.size(); i++){
        int key = keys[i];
        QString text = pInstance->getText(key);
        if(AbstractInputMethod::isLetter(key)){
            bBigLetter ?  text =text.toUpper() :  text =text.toLower();
        }
        buttons[i]->set(text,  key);
    }
}

void KeyboardWidgetPrivate::updateState()
{
    switch (currentPage) {
    case 0:
        this->updatPage(0);
        break;
    case 1:
        this->updatPage(2);
        break;
    case 2:
        this->updatPage(1);
        break;
    default:
        break;
    }
}


class  Keyboard: public QWidget
{
public:
    Keyboard(QWidget *parent =nullptr):QWidget(parent){}

    void paintEvent(QPaintEvent *event){
        QWidget::paintEvent(event);
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }
};
int KeyboardWidgetPrivate::currentPage =0;

KeyboardWidget::KeyboardWidget(QObject *parent) :
    QObject(parent),
    m_d(new KeyboardWidgetPrivate)
{

    //page1
    int letterPage[PAGEKEYCOUNT]={Qt::Key_Q, Qt::Key_W, Qt::Key_E, Qt::Key_R, Qt::Key_T, Qt::Key_Y, Qt::Key_U, Qt::Key_I, Qt::Key_O, Qt::Key_P, Qt::Key_Backspace, //11
                                  Qt::Key_A, Qt::Key_S, Qt::Key_D, Qt::Key_F, Qt::Key_G, Qt::Key_H, Qt::Key_J, Qt::Key_K, Qt::Key_L , Qt::Key_Return, //10
                                  Qt::Key_Shift, Qt::Key_Z, Qt::Key_X, Qt::Key_C, Qt::Key_V, Qt::Key_B, Qt::Key_N, Qt::Key_M, Qt::Key_Period, Qt::Key_Comma, Qt::Key_Shift, //11
                                  PageKey , NotUsedKey, NotUsedKey,  LanguageKey, NotUsedKey, NotUsedKey, CloseKey //7
                                 };
    //page2
    int numberPage[PAGEKEYCOUNT]={ Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,Qt::Key_6,Qt::Key_7,Qt::Key_8,Qt::Key_9, Qt::Key_0, Qt::Key_Backspace, //11
                                   Qt::Key_At, Qt::Key_NumberSign,  Qt::Key_Percent, Qt::Key_Ampersand, Qt::Key_Asterisk, Qt::Key_Minus, Qt::Key_Plus, Qt::Key_ParenLeft, Qt::Key_ParenRight, Qt::Key_Return,//10
                                   Qt::Key_Shift, Qt::Key_Exclam, Qt::Key_QuoteDbl, Qt::Key_Less, Qt::Key_Greater, Qt::Key_Apostrophe, Qt::Key_Colon, Qt::Key_Semicolon, Qt::Key_Period, Qt::Key_Comma,Qt::Key_Shift,//11
                                   PageKey , NotUsedKey, NotUsedKey,  LanguageKey, NotUsedKey, NotUsedKey, CloseKey //7
                                 };
    //page3
    int symbolPage[PAGEKEYCOUNT]={ Qt::Key_1,Qt::Key_2,Qt::Key_3,Qt::Key_4,Qt::Key_5,Qt::Key_6,Qt::Key_7,Qt::Key_8,Qt::Key_9, Qt::Key_0, Qt::Key_Backspace, //11
                                   Qt::Key_QuoteLeft, Qt::Key_AsciiTilde, Qt::Key_Dollar, Qt::Key_AsciiCircum, Qt::Key_Underscore,Qt::Key_Equal, Qt::Key_BraceLeft, Qt::Key_BraceRight, Qt::Key_BracketLeft, Qt::Key_Return,//10
                                   Qt::Key_Shift, Qt::Key_BracketRight, Qt::Key_Bar, Qt::Key_Backslash, Qt::Key_Slash, Qt::Key_Question ,Qt::Key_Period, Qt::Key_Comma,  NotUsedKey,NotUsedKey, Qt::Key_Shift,//11
                                   PageKey , NotUsedKey, NotUsedKey,  LanguageKey, NotUsedKey, NotUsedKey, CloseKey //7
                                 };

    m_d->keyboardWidget = new Keyboard;
    m_d->keyboardWidget->setObjectName("im_keyboardwidget");

    QVBoxLayout *vBox=new QVBoxLayout(m_d->keyboardWidget);
    m_d->listView = new QListView;
    m_d->listView->setObjectName("candidateView");
    connect(m_d->listView, &QListView::clicked, this, [=](const QModelIndex &index){
        QString text =index.data().toString();
        InputMethodManager::instance()->curentInputMethod()->commit(text);
    });
    m_d->listView->setInputMethodHints(Qt::ImhNone);;
    m_d->listView->setFocusPolicy(Qt::NoFocus);
    m_d->listView->setStyleSheet("border:none;");
    m_d->listView->setEditTriggers(QListView::NoEditTriggers);
    m_d->listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_d->listView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_d->listView->setFlow(QListView::LeftToRight);
    m_d->listView->setContentsMargins(0,0,0,0);
    m_d->listView->setModel(&m_d->model);
    m_d->listView->setSpacing(6);

    m_d->listView->setMinimumHeight(20);
    m_d->listView->setMaximumHeight(35);

    QMargins marging = vBox->contentsMargins();
    vBox->setContentsMargins(marging.left(), 5, marging.right(), marging.bottom());
    vBox->addWidget(m_d->listView);
    vBox->setSpacing(0);
    QGridLayout *gLayout = new QGridLayout;
    if(qApp->primaryScreen()->size().width() <1280)
          gLayout->setSpacing(6);
    else
        gLayout->setSpacing(10);
    vBox->addLayout(gLayout);

    int span =1;
    int count = 0;
    for(int i=0; i<PAGEKEYCOUNT; i++){
        ButtonKey *btn = new ButtonKey;
        btn->set("",letterPage[i] );
        m_d->buttons <<btn;
        int key = letterPage[i];

        if(key == Qt::Key_Backspace)
        {
            btn->setAutoRepeat(true);
        }

        switch (key) {
        case PageKey:  //切换页面.
            btn->setObjectName("key_page"); span = 1;
            connect(btn, &ButtonKey::clicked, this, [=](){
                m_d->currentPage == 0 ? m_d->currentPage =2: m_d->currentPage =0;
                m_d->updateState();
            });
            break;
        case Qt::Key_Shift: //大小写.
            btn->setObjectName("key_shift"); span = 1;
            connect(btn, &ButtonKey::clicked,  [=](){
                m_d->bBigLetter = !m_d->bBigLetter;
                m_d->updateState();
            });
            break;

        case LanguageKey:  //语言切换按键.
            btn->setObjectName("key_language"); span = 5;
            connect(btn, &ButtonKey::clicked, this, [=](){
                if(InputMethodManager::instance()->curentInputMethod()->language() == QLocale::Chinese){
                    IMInputContext::instance()->commit();
                }
                InputMethodManager::instance()->changeLanguage();
                m_d->updatPage();
            });
            break;

        case CloseKey: //关闭按键
        {
            btn->setObjectName("key_close"); span = 1;
            connect(btn, &ButtonKey::clicked, this, [=](){
                IMInputContext::instance()->commit();
                this->hide();
            });
        }
            break;

        case NotUsedKey: //不使用的按键，占位用途.
            btn->setObjectName("key_empty"); span = 1;
            btn->setEnabled(false);
            break;

        default:
            span =1; btn->setObjectName("key_common");
            connect(btn, &ButtonKey::pressed, this, [=](){
                QString text = btn->text();
                if(InputMethodManager::instance()->curentInputMethod()->language() == QLocale::Chinese){
                    text = text.toLower();
                }
                QKeyEvent e(QEvent::KeyPress, btn->key(), Qt::NoModifier, text);
                IMInputContext::instance()->eventFilter(nullptr, &e);
            });

            if(key == Qt::Key_Return){
                span = 2;
                btn->setObjectName("key_enter");
            }
            break;
        }

        gLayout->addWidget(btn, count/11, count%11, 1, span);
        count +=span;

        //保存页按键，用于切换处理.
        m_d->pageKeys1 << letterPage[i];
        m_d->pageKeys2 << numberPage[i];
        m_d->pageKeys3 << symbolPage[i];
    }

    m_d->updatPage(0);
    connect(IMInputContext::instance(), &IMInputContext::dataChange, this, [=](const QStringList &model){
        m_d->model.setStringList(model);
    });


    //platform set.
    if(qApp->platformName() =="xcb" || qApp->platformName() == "eglfs"){
        m_d->keyboardWidget->setWindowFlags( Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint);
        m_d->bSupport = true;

        if(qApp->platformName() == "eglfs")
        {
            qApp->installEventFilter(this);
        }
    }else{
        m_d->keyboardWidget->setFocusPolicy(Qt::NoFocus);
        connect(qApp, &QApplication::focusChanged, [=](QWidget *old, QWidget *now){


            //remove parent-child .
           if(m_d->keyboardWidget && m_d->keyboardWidget->parentWidget()){
              m_d->keyboardWidget->parentWidget()->removeEventFilter(this);
               m_d->keyboardWidget->setParent(NULL);
            }

            //qDebug()<<"old = " << old <<" now="<<now <<"keyboardWidget=" << m_d->keyboardWidget;
            if(now && now != m_d->keyboardWidget && !now->isModal()){;
               QWidget *topWidget = now->topLevelWidget();
                if(topWidget){
                    QSize size = qApp->primaryScreen()->size();
                    m_d->bSupport=topWidget->isFullScreen()
                            || topWidget->isMaximized()
                            ||(size.width() <= topWidget->size().width() && size.height() <=topWidget->height() );
                    //qDebug()<<topWidget << m_d->bSupport;
                }else{
                    m_d->bSupport = false;
                }

                //set parent-child.
                if(m_d->bSupport && topWidget){
                    m_d->bSupport = true;
                    topWidget->installEventFilter(this);
                    m_d->keyboardWidget->setParent(topWidget);
                }
            }
        });
    }

    //use imkeyboard.style.
    QString fileName =qApp->applicationDirPath() +"/imkeyboard.style";
    QString styleSheet;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly| QFile::Text)){
        styleSheet = file.readAll().trimmed();
        file.close();
    }

    //default style.
    if(styleSheet.isEmpty()){
        m_d->keyboardWidget->setStyleSheet("QListView{color: green; font: bold 18px; background:rgb(35,35,35);}"
                                           " QPushButton{background: rgb(55,55,55); color: white; font-size: 18px;} "
                                           "QPushButton:pressed{background: rgb(175,175,175); color: white; font-size: 18px;}"
                                           "QWidget#im_keyboardwidget{background:rgb(35,35,35)}");
    }else{
        m_d->keyboardWidget->setStyleSheet(styleSheet);
    }
    connect(qApp, &QApplication::aboutToQuit, this, [=](){
        m_d->keyboardWidget->close();
        delete m_d->keyboardWidget;
        m_d->keyboardWidget = nullptr;
    });

}

KeyboardWidget::~KeyboardWidget()
{
    if(m_d->keyboardWidget)
    {
        m_d->keyboardWidget->close();
        delete m_d->keyboardWidget;
        m_d->keyboardWidget = nullptr;
    }
    delete m_d;
}

void KeyboardWidget::show()
{
    if((qApp->platformName() == "wayland") && (m_d->keyboardWidget->parentWidget() == 0))
        return;

    if(qApp->focusWindow() &&!qApp->focusWindow()->isModal() && m_d->bSupport)
    {
        if(qApp->platformName() !="xcb")
        {
        //在点击QTableView QTableWidget 必须隐藏键盘弹出，代理项双击编辑时在显示，
        if(qApp->focusObject() && qApp->focusObject()->inherits("QAbstractItemView")){
            this->hide();
            return;
          }
        }
        m_d->keyboardWidget->show();
        this->resize();
    }
}

void KeyboardWidget::hide()
{
    m_d->keyboardWidget->hide();
}

bool KeyboardWidget::isVisible()
{
    return m_d->keyboardWidget && m_d->keyboardWidget->isVisible();
}

void KeyboardWidget::resize()
{
    QTransform t = QGuiApplication::inputMethod()->inputItemTransform();

    IMPlatformInputContext *platCtx = IMInputContext::instance()->platformCtx();

    QRect keyboardRect;
    if( platCtx && qApp->focusWindow()){
        QRect cusRext = platCtx->inputMethodQuery(Qt::ImCursorRectangle).toRect();
        QRect gCursorFocusWindowRect = t.mapRect(cusRext);
        QPoint gWindowPos =qApp->focusWindow()->mapToGlobal(QPoint(0,0));

        int gCurorX = gWindowPos.x() + gCursorFocusWindowRect.x() + gCursorFocusWindowRect.width();
        int gCurorY = gWindowPos.y() + gCursorFocusWindowRect.y() + gCursorFocusWindowRect.height();
        QPoint gloabePos(gCurorX, gCurorY);

        if(qApp->primaryScreen() && m_d->keyboardWidget->parent() == NULL){
            m_d->rect = qApp->primaryScreen()->availableGeometry();
        }else if(qApp->activeWindow()){
            m_d->rect = qApp->activeWindow()->geometry();
        }

        int width =   m_d->rect.width()*3/4;
        int height = qCeil(width*1/3.0);

        //eglfs 旋转后调整显示比例.
        if(m_d->rect.width() <m_d->rect.height())
        {
            width = m_d->rect.width();
            height = width*2/5.0;
        }

        if(!qApp->property("bottom").toBool())
        {
            int maxX = m_d->rect.width() -width;
            int maxY = m_d->rect.height()-height;
            if(gloabePos.x() > maxX){
                gloabePos.setX(maxX);
            }

            if(gloabePos.y() >maxY){
                gloabePos.setY(gCurorY-height-cusRext.height());
            }else{
                gloabePos.setY(gloabePos.y());
            }

            keyboardRect.setX(gloabePos.x());
            keyboardRect.setY(gloabePos.y());
            keyboardRect.setWidth(width);
            keyboardRect.setHeight(height);

        }else{
            keyboardRect.setX((m_d->rect.width()- width)/2);
            keyboardRect.setY(m_d->rect.height()-height);
            keyboardRect.setWidth(width);
            keyboardRect.setHeight(height);
        }
    }
    m_d->keyboardWidget->setGeometry(keyboardRect);
}

bool KeyboardWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(qApp->platformName() =="eglfs")
     {
         if(event->type() == QEvent::FocusIn){
             QFocusEvent* inEvent =dynamic_cast<QFocusEvent*>(event);
             m_d->bSupport =inEvent->reason() == Qt::MouseFocusReason;
         }else if(event->type() == QEvent::MouseButtonPress){
             m_d->bSupport = true; 
         }
    }
    return QObject::eventFilter(watched, event);
}


ButtonKey::ButtonKey( QWidget *parent):
    QPushButton(parent),
    m_text("text"),
    m_key(-1)
{
    this->setFocusPolicy(Qt::NoFocus);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setMinimumSize(20, 20);
}

void ButtonKey::set(const QString &text, int key){
    m_text = text;
    m_key = key;
    m_showText = m_text;
    switch (m_key) {
    case Qt::Key_Return:
        m_showText = "Enter";
        break;

    case Qt::Key_Backspace:
        m_showText = "Del";
        break;

    case Qt::Key_Shift:
        switch (KeyboardWidgetPrivate::currentPage) {
        case 0:
            m_showText = "Caps";
            break;
        case 1:
            m_showText = "1/2";
            break;
        case 2:
            m_showText = "2/2";
            break;
        default:
            break;
        }
        break;

    case Qt::Key_Ampersand:
        m_showText ="&&";
        break;

    case PageKey:
        m_showText = "&&123";
        break;

    case LanguageKey:
        if(InputMethodManager::instance()->curentInputMethod()->language() == QLocale::English){
            m_showText = "English";
        }else if(InputMethodManager::instance()->curentInputMethod()->language() == QLocale::Chinese){
            m_showText="中文";
        }
        break;

    case CloseKey:
        m_showText = "x";
        break;
    default:
        break;
    }
    this->setText(m_showText);
}

int ButtonKey::key()
{
    return m_key;
}

QString ButtonKey::text()
{
    return m_text;
}

