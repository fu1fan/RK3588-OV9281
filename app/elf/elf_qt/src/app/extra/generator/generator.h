#ifndef GENERATOR_H
#define GENERATOR_H
#include <QString>
#include <QObject>

class Generator : public QObject
{
public:
    Generator(QObject * parent = nullptr);
    ~ Generator();

    static void initAppBefore(int argc, char *argv[]);
    static void  create(const QString &name);

private:
    bool eventFilter(QObject *watched, QEvent *event) override;
    QList<QObject*> m_objects;
};

#endif // GENERATOR_H
