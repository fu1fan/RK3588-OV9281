#ifndef BACKLIGHTWIDGET_H
#define BACKLIGHTWIDGET_H

#include <QWidget>

namespace Ui {
class BacklightWidget;
}

class  BacklightWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BacklightWidget(QWidget *parent = nullptr);
    ~BacklightWidget();
    void resetTr();

public slots:
    void currentBacklightChange(int index);

private:
    void createColowWidget();
    Ui::BacklightWidget *ui;
};

#endif // BACKLIGHTWIDGET_H
