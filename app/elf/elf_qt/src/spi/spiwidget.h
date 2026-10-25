#ifndef SPIWIDGET_H
#define SPIWIDGET_H

#include <QWidget>

namespace Ui {
class SpiWidget;
}

class SpiWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SpiWidget(QWidget *parent = nullptr);
    ~SpiWidget();

    virtual void resetTr();

public slots:
    void currentSpiChange(int id);

private slots:
    void on_clearBtn_clicked();

private:
    Ui::SpiWidget *ui;
};

#endif // SPIWIDGET_H
