#ifndef M4GWIDGET_H
#define M4GWIDGET_H

#include <QWidget>

namespace Ui {
class M4gWidget;
}

class QTimer;
class  M4gWidget : public QWidget
{
    Q_OBJECT

public:
    explicit M4gWidget(QWidget *parent = nullptr);
    ~M4gWidget();
    void resetTr();
    void clearUi();

public slots:
    void on_connectBtn_clicked();
    void readInformation();
    void on_applyBtn_clicked();

protected:
    Ui::M4gWidget *ui;
    QTimer *m_timer;
};

#endif // M4GWIDGET_H
