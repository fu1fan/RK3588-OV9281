#ifndef GLWIDGET_H
#define GLWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTime>
#include <QVector>
#include <QPushButton>

QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class OpenglWidget: public QWidget
{
public:
     OpenglWidget(QWidget *parent = nullptr);
};

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget( const QColor &background);
    ~GLWidget();

public slots:
    void loadAscllStl(QString filename, qreal ratio);

protected:
    void paintGL() override;
    void initializeGL() override;

private:
    void paintFLLogo();
    void createGeometry();

    qreal m_fAngle;
    qreal m_fScale;
    QVector<QVector3D> m_vertices;
    QVector<QVector3D> m_normals;
    int m_frames;
    QTime m_time;
    QOpenGLShader *m_vshader1;
    QOpenGLShader *m_fshader1;
    QOpenGLShaderProgram *m_program1;
    QOpenGLBuffer m_vbo1;
    int m_vertexAttr1;
    int m_normalAttr1;
    int m_matrixUniform1;
    bool m_transparent;
    QColor m_background;
};

#endif
