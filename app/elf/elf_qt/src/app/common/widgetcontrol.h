//窗口管理控制接口，
#ifndef WIDGETCONTROL_H
#define WIDGETCONTROL_H
#include "processhandler.h"

class MainControl;

//小部件接口.
class WidgetControl
{
public:
    //翻译接口，子类重现实现
    virtual void resetTr();

    //主窗口控制器实例.
    MainControl *control();
};

//主窗口控制接口.
class MainControl : public WidgetControl
{
public:
    //设置窗口标题可见，
    virtual void setWindowTitleVisible(bool bVisible, int border = 10) = 0;

    //显示某个小部件到主窗口.
    virtual void showWidget(int id) = 0;

    //当前主窗口显示的窗口id
    virtual int currentWidget() = 0;

private:
    MainControl *control() = delete;
};

//继承式方便，
#define WIDGETCONTROL ,public WidgetControl
//继承是方便.添加了进程处理接口，用于出命令交互令读写
#define WIDGETCONTROLPROGRESS ,public WidgetControl, protected fl_common::ProcessHandler

#define MAINCONTROL ,public MainControl
#endif // WIDGETCONTROL_H
