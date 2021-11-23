#include "glwindow.h"

#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShader>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLTexture>
#include <QImage>
#include <QMouseEvent>

GLWindow::GLWindow()
{
    m_time = QDateTime::currentDateTime();
    connect(this, &QOpenGLWindow::frameSwapped, this, [this] {
        QDateTime newTime = QDateTime::currentDateTime();
        u_dt = (newTime.toMSecsSinceEpoch() - m_time.toMSecsSinceEpoch()) * 0.001;
        u_time += u_dt;
        this->update();
        m_time = newTime;
        //qDebug() << 1. / u_dt;
    });
}

GLWindow::~GLWindow()
{
    makeCurrent();
    delete m_renderProgram;
    delete m_simulationProgram;
    delete m_vbo;
    delete m_vao;
    delete m_fbo;
    delete m_textureBaseColor;
    delete m_textureEnvironmentMap;
}

void GLWindow::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_openGlPaintDevice = new QOpenGLPaintDevice(width(), height());

    m_simulationProgram = new QOpenGLShaderProgram;
    m_simulationProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_simulationProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/simulation.fsh");
    m_simulationProgram->link();

    m_renderProgram = new QOpenGLShaderProgram;
    m_renderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_renderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/frag.fsh");
    m_renderProgram->link();

    m_vao = new QOpenGLVertexArrayObject;
    if (m_vao->create())
        m_vao->bind();

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo->bind();
    m_vbo->allocate(m_quad, sizeof(m_quad));

    QOpenGLFramebufferObjectFormat format = QOpenGLFramebufferObjectFormat();
    format.setInternalTextureFormat(QOpenGLTexture::RG32F);
    m_fbo = new QOpenGLFramebufferObject(m_simSize.width(), m_simSize.height(), format);
    m_fbo->bind();

    m_textureBaseColor = new QOpenGLTexture(QImage(":/textures/QT.png"));
    m_textureBaseColor->create();

    m_textureEnvironmentMap = new QOpenGLTexture(QImage(":/textures/environment_map.png"));
    m_textureEnvironmentMap->create();

    m_renderProgram->enableAttributeArray(0);
    m_renderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_simulationProgram->enableAttributeArray(0);
    m_simulationProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

    m_vbo->release();
    m_vao->release();
    m_fbo->release();
}

void GLWindow::resizeGL(int /*w*/, int /*h*/)
{

}

void GLWindow::setUniforms(QOpenGLShaderProgram *program)
{
    program->setUniformValue("uDt", u_dt);
    program->setUniformValue("uTime", u_time);
    program->setUniformValue("uResolution", width(), height());
    program->setUniformValue("uTexelSize", 1.0f / m_simSize.width(), 1.0f / m_simSize.height(), 0.0f);
}

void GLWindow::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT);
    m_renderProgram->release();

    // Simulation
    f->glViewport(0, 0, m_simSize.width(), m_simSize.height());
    m_simulationProgram->bind();
    m_vao->bind();

    f->glBindTexture(QOpenGLTexture::Target2D, m_fbo->texture());

    setUniforms(m_simulationProgram);

    m_fbo->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fbo->release();

    m_simulationProgram->release();

    // Render
    f->glViewport(0, 0, width(), height());
    m_renderProgram->bind();

    f->glActiveTexture(GL_TEXTURE0);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "simulationTexture"), 0);

    f->glActiveTexture(GL_TEXTURE1);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glBindTexture(GL_TEXTURE_2D, m_textureBaseColor->textureId());
    f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "albedoTexture"), 1);

    f->glActiveTexture(GL_TEXTURE2);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glBindTexture(GL_TEXTURE_2D, m_textureEnvironmentMap->textureId());
    f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "environmentTexture"), 2);

    setUniforms(m_renderProgram);

    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_vao->release();
}

void GLWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_pressed) return;

    QPainter painter = QPainter();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT);

    m_fbo->bind();
    QPoint p = QPoint(0.0, height() - m_simSize.height());

    painter.begin(m_openGlPaintDevice);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(0, 20, 0, 255), 5));
    painter.setBrush(QImage(":/textures/brush.png"));

    painter.drawLine(m_lastPoint * (m_simSize.width() / (float)width()) + p,event->position() * (m_simSize.height() / (float)height()) + p);
    painter.end();

    m_lastPoint = event->position();

    m_fbo->release();
}

void GLWindow::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_lastPoint = event->position();
}

void GLWindow::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
}
