#include "glwidget.h"
#include <QPainter>
#include <QPaintEngine>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <qmath.h>
#include <QFile>
#include "resourceloader.h"
#include <QHBoxLayout>
#include <QtDebug>
#include <QScreen>
#include <QApplication>

#ifndef GL_SRGB8_ALPHA8
#define GL_SRGB8_ALPHA8 0x8C43
#endif

GLWidget::GLWidget( const QColor &background)
    :
      m_frames(0),
      m_program1(nullptr),
      m_transparent(false),
      m_background(background)
{
    //setTextureFormat(GL_SRGB8_ALPHA8);
}

GLWidget::~GLWidget()
{
    // And now release all OpenGL resources.
    makeCurrent();
    delete m_program1;
    delete m_vshader1;
    delete m_fshader1;
    m_vbo1.destroy();
    doneCurrent();
}

void GLWidget::paintFLLogo()
{
    m_program1->enableAttributeArray(m_vertexAttr1);
    m_program1->enableAttributeArray(m_normalAttr1);

    m_vbo1.bind();
    // The data in the buffer is placed like this:
    // vertex1.x, vertex1.y, vertex1.z, normal1.x, normal1.y, normal1.z, vertex2.x, ...
    m_program1->setAttributeBuffer(m_vertexAttr1, GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    m_program1->setAttributeBuffer(m_normalAttr1, GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));
    m_vbo1.release();

    glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

    m_program1->disableAttributeArray(m_normalAttr1);
    m_program1->disableAttributeArray(m_vertexAttr1);
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_vshader1 = new QOpenGLShader(QOpenGLShader::Vertex);
    const char *vsrc1 =
            "attribute highp vec4 vertex;\n"
            "attribute mediump vec3 normal;\n"
            "uniform mediump mat4 matrix;\n"
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));\n"
            "    float angle = max(dot(normal, toLight), 0.0);\n"
            "    vec3 col = vec3(0.0, 0.87, 2.66);\n"
            "    color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
            "    color = clamp(color, 0.0, 1.0);\n"
            "    gl_Position = matrix * vertex;\n"
            "}\n";
    m_vshader1->compileSourceCode(vsrc1);

    m_fshader1 = new QOpenGLShader(QOpenGLShader::Fragment);
    const char *fsrc1 =
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = color;\n"
            "}\n";
    m_fshader1->compileSourceCode(fsrc1);

    m_program1 = new QOpenGLShaderProgram;
    m_program1->addShader(m_vshader1);
    m_program1->addShader(m_fshader1);
    m_program1->link();

    m_vertexAttr1 = m_program1->attributeLocation("vertex");
    m_normalAttr1 = m_program1->attributeLocation("normal");
    m_matrixUniform1 = m_program1->uniformLocation("matrix");

    m_fAngle = 0;
    m_fScale = 1.2;

    createGeometry();

    // Use a vertex buffer object. Client-side pointers are old-school and should be avoided.
    m_vbo1.create();
    m_vbo1.bind();
    // For the cube all the data belonging to the texture coordinates and
    // normals is placed separately, after the vertices. Here, for the Qt logo,
    // let's do something different and potentially more efficient: create a
    // properly interleaved data set.
    const int vertexCount = m_vertices.count();
    QVector<GLfloat> buf;
    buf.resize(vertexCount * 3 * 2);
    GLfloat *p = buf.data();
    for (int i = 0; i < vertexCount; ++i) {
        *p++ = m_vertices[i].x();
        *p++ = m_vertices[i].y();
        *p++ = m_vertices[i].z();
        *p++ = m_normals[i].x();
        *p++ = m_normals[i].y();
        *p++ = m_normals[i].z();
    }
    m_vbo1.allocate(buf.constData(), buf.count() * sizeof(GLfloat));
    m_vbo1.release();
}

void GLWidget::paintGL()
{
    QPainter painter;
    painter.begin(this);

    painter.beginNativePainting();

    glClearColor(m_background.redF(), m_background.greenF(), m_background.blueF(), m_transparent ? 0.0f : 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    QMatrix4x4 modelview;
    modelview.rotate(m_fAngle, 0.0f, 1.0f, 0.0f);
    modelview.rotate(m_fAngle, 1.0f, 0.0f, 0.0f);
    modelview.rotate(m_fAngle, 0.0f, 0.0f, 1.0f);
    modelview.scale(m_fScale);
    modelview.translate(0.0f, -0.2f, 0.0f);


    m_program1->bind();
    m_program1->setUniformValue(m_matrixUniform1, modelview);
    paintFLLogo();
    m_program1->release();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();

    if (const int elapsed = m_time.elapsed()) {
        QString framesPerSecond;
        framesPerSecond.setNum(m_frames /(elapsed / 1000.0), 'f', 2);
        painter.setPen(Qt::blue);
        painter.drawText(20, 40, framesPerSecond + " paintGL calls / s");
    }

    painter.end();


    if (!(m_frames % 100)) {
        m_time.start();
        m_frames = 0;
    }
    m_fAngle += 1.0f;
    ++m_frames;

    update();
}

void GLWidget::loadAscllStl(QString filename, qreal ratio)
{
    QVector3D Normal;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << QString("open  file %1 falire").arg(filename);
        return;
    }
    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();
        QStringList  words = line.split(' ', QString::SkipEmptyParts);
        if (words[0] == "facet") {
            Normal = QVector3D(ratio*words[2].toFloat(), ratio*words[3].toFloat(),ratio*words[4].toFloat());
        }
        else if (words[0] == "vertex") {
            m_vertices << QVector3D(ratio*words[1].toFloat(), ratio*words[2].toFloat(),ratio*words[3].toFloat());
            m_normals <<Normal;

        }
    }
}

void GLWidget::createGeometry()
{
    m_vertices.clear();
    m_normals.clear();
    QString stlFileName = fl_common::ResourceLoader::instacne()->rc("opengl/logo-3"); //:/logo-3.STL
    this->loadAscllStl(stlFileName, 0.1);
}

OpenglWidget::OpenglWidget(QWidget *parent):QWidget(parent)
{
   if(QScreen *pScreen =qApp->primaryScreen())
   {
       int w = pScreen->availableGeometry().width();
       int h = pScreen->availableGeometry().height();
        QBoxLayout *hLayout;
       if(w >h){
           hLayout = new QHBoxLayout(this);
       }else{
           hLayout = new QVBoxLayout(this);
       }
       hLayout->addWidget(new GLWidget("white"));
       hLayout->addWidget(new GLWidget("white"));

   }
}

