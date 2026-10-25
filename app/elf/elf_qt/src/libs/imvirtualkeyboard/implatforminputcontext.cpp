#include "implatforminputcontext.h"
#include "iminputcontext.h"
#include <QInputMethodQueryEvent>
#include <QtDebug>
#include <QGuiApplication>
#include "iminputcontext.h"
#include <QWindow>
#include "inputmethodmanager.h"
#include "keyboardwidget.h"

IMPlatformInputContext::IMPlatformInputContext() :
    m_inputContext(new IMInputContext(this)),
    m_focusObject(NULL),
    m_keyboard(NULL),
    m_bVisible(false)
{
    IMInputContext::instance()->setPlatformCtx(this);
}

IMPlatformInputContext::~IMPlatformInputContext()
{
 if(m_keyboard)
 {
     delete m_keyboard;
     m_keyboard = nullptr;
 }
}

bool IMPlatformInputContext::isValid() const
{
    return true;
}

void IMPlatformInputContext::commit()
{
   IMInputContext::instance()->commit();
}

void IMPlatformInputContext::showInputPanel()
{
    if(m_keyboard == NULL){
        m_keyboard = new KeyboardWidget;
    }

    if(m_focusObject && !m_keyboard->isVisible()){
        //qDebug()<<"showInputPanel";
        m_keyboard->show();
        m_bVisible = true;
    }

}

void IMPlatformInputContext::hideInputPanel()
{
    if(m_keyboard != NULL && m_keyboard->isVisible()){
        //qDebug()<<"hide hideInputPanel";
        m_keyboard->hide();
        m_bVisible = false;
    }
}

bool IMPlatformInputContext::isInputPanelVisible() const
{
    return m_keyboard && m_keyboard->isVisible();
}

void IMPlatformInputContext::setFocusObject(QObject *object)
{
    if(inputMethodAccepted() &&object){
        m_focusObject = object;
        //qDebug()<<"setFocusObject" <<object;
    }else{
        m_focusObject = nullptr;
        // qDebug()<<"setFocusObject2" <<object;
    }
}


QObject *IMPlatformInputContext::focusObject()
{
    return m_focusObject;
}

bool IMPlatformInputContext::isVisble()
{
    return m_keyboard && m_keyboard->isVisible();
}

QVariant IMPlatformInputContext::inputMethodQuery(Qt::InputMethodQuery query)
{
    QInputMethodQueryEvent event(query);
    if(m_focusObject){
        qApp->sendEvent(m_focusObject,  &event);
    }
    return event.value(query);
}

void IMPlatformInputContext::update(Qt::InputMethodQueries)
{
    //qDebug()<<m_focusObject;
   if(inputMethodQuery(Qt::ImEnabled).toBool())
   {
       if(!isVisble() && m_bVisible)
       this->showInputPanel();
       //qDebug()<<"show";
   }else{

       if(isVisble() && !m_bVisible)
       this->hideInputPanel();
       // qDebug()<<"hide";
   }
}

