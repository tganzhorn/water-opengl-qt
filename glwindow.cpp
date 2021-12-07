#include "glwindow.h"

#include <QMouseEvent>
#include <QWheelEvent>
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
#include <QColorSpace>

GLWindow::GLWindow()
{
    m_time = QDateTime::currentDateTime();
    connect(this, &QOpenGLWindow::frameSwapped, this, [this] {
        QDateTime newTime = QDateTime::currentDateTime();
        u_dt = (newTime.toMSecsSinceEpoch() - m_time.toMSecsSinceEpoch()) * 0.001;
        u_time += u_dt;
        this->update();
        m_time = newTime;
    });
}

GLWindow::~GLWindow()
{
    makeCurrent();
    delete m_renderProgram;
    delete m_simulationProgram;
    delete m_interactionProgram;
    delete m_vbo;
    delete m_vao;
    delete m_fboSimulation;
    delete m_fboNormals;
    delete m_fboCaustics;
    delete m_textureBaseColor;
    delete m_textureEnvironmentMap;
    delete m_normalsProgram;
    delete m_causticsProgram;
    delete m_demoProgram;
}

void GLWindow::initializeGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    m_openGlPaintDevice = new QOpenGLPaintDevice(width(), height());

    f->glDisable(GL_FRAMEBUFFER_SRGB);
    defaultFramebufferObject();

    m_simulationProgram = new QOpenGLShaderProgram;
    m_simulationProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_simulationProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/simulation.fsh");
    m_simulationProgram->link();

    m_renderProgram = new QOpenGLShaderProgram;
    m_renderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_renderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/frag.fsh");
    m_renderProgram->link();

    m_interactionProgram = new QOpenGLShaderProgram;
    m_interactionProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_interactionProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/interaction.fsh");
    m_interactionProgram->link();

    m_normalsProgram = new QOpenGLShaderProgram;
    m_normalsProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_normalsProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/normals.fsh");
    m_normalsProgram->link();

    m_causticsProgram = new QOpenGLShaderProgram;
    m_causticsProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_causticsProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/caustics.fsh");
    m_causticsProgram->link();

    m_demoProgram = new QOpenGLShaderProgram;
    m_demoProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":shaders/vert.vsh");
    m_demoProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":shaders/demo.fsh");
    m_demoProgram->link();

    m_vao = new QOpenGLVertexArrayObject;
    if (m_vao->create())
        m_vao->bind();

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo->bind();
    m_vbo->allocate(m_quad, sizeof(m_quad));

    QOpenGLFramebufferObjectFormat formatSimulation = QOpenGLFramebufferObjectFormat();
    formatSimulation.setInternalTextureFormat(QOpenGLTexture::RG32F);
    m_fboSimulation = new QOpenGLFramebufferObject(m_simSize.width(), m_simSize.height(), formatSimulation);
    m_fboSimulation->bind();

    QOpenGLFramebufferObjectFormat formatNormals = QOpenGLFramebufferObjectFormat();
    formatNormals.setInternalTextureFormat(QOpenGLTexture::RGB16F);
    formatNormals.setTextureTarget(GL_TEXTURE_2D);
    m_fboNormals = new QOpenGLFramebufferObject(m_simSize.width(), m_simSize.height(), formatNormals);
    m_fboNormals->bind();

    QOpenGLFramebufferObjectFormat formatCaustics = QOpenGLFramebufferObjectFormat();
    formatCaustics.setInternalTextureFormat(QOpenGLTexture::RGB32F);
    m_fboCaustics = new QOpenGLFramebufferObject(width(), height(), formatCaustics);
    m_fboCaustics->bind();

    QImage x = QImage(":/textures/QT.png");
    x.convertToColorSpace(QColorSpace(QColorSpace::SRgbLinear));

    m_textureBaseColor = new QOpenGLTexture(x);
    m_textureBaseColor->create();

    m_textureEnvironmentMap = new QOpenGLTexture(QImage(":/textures/environment_map.png"));
    m_textureEnvironmentMap->create();

    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

    m_vbo->release();
    m_vao->release();
    m_fboSimulation->release();
}

void GLWindow::resizeGL(int /*w*/, int /*h*/)
{

}

void GLWindow::setUniforms(QOpenGLShaderProgram *program)
{
    program->setUniformValue("uDt", u_dt);
    program->setUniformValue("uTime", u_time);
    program->setUniformValue("uResolution", width(), height());
    program->setUniformValue("uSimTexelSize", 1.0f / m_simSize.width(), 1.0f / m_simSize.height());
    program->setUniformValue("uRenderTexelSize", 1.0f / width(), 1.0f / height());
    program->setUniformValue("uK", u_k);
    program->setUniformValue("uDamping", u_damping);
    program->setUniformValue("uNormalStrength", u_normalStrength);
    program->setUniformValue("uLightStrength", u_lightStrength);
    program->setUniformValue("uLightPosition", u_lightPosition);
    program->setUniformValue("uPlanePosition", u_planePosition);
    program->setUniformValue("uPlaneNormal", u_planeNormal);
    program->setUniformValue("uCameraPosition", u_cameraPosition);
}

void GLWindow::setTextureFilter(QOpenGLFunctions *f)
{
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void GLWindow::paintGL()
{
    // Initialization and cleanup
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT);

    // Simulation pass
    // Calculation happens at sim size
    f->glViewport(0, 0, m_simSize.width(), m_simSize.height());
    m_simulationProgram->bind();
    m_vao->bind();

    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(QOpenGLTexture::Target2D, m_fboSimulation->texture());
    setTextureFilter(f);
    f->glUniform1i(f->glGetUniformLocation(m_simulationProgram->programId(), "texture"), 0);

    setUniforms(m_simulationProgram);

    m_fboSimulation->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fboSimulation->release();

    m_simulationProgram->release();

    // Normals pass
    // Calculation happens at sim size
    f->glViewport(0, 0, m_simSize.width(), m_simSize.height());
    m_normalsProgram->bind();

    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(QOpenGLTexture::Target2D, m_fboSimulation->texture());
    setTextureFilter(f);
    f->glUniform1i(f->glGetUniformLocation(m_normalsProgram->programId(), "texture"), 0);

    setUniforms(m_normalsProgram);

    m_fboNormals->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fboNormals->release();

    m_normalsProgram->release();

    // Caustics pass
    // Calculation happens at render size
    f->glViewport(0, 0, width(), height());
    m_causticsProgram->bind();

    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(QOpenGLTexture::Target2D, m_fboSimulation->texture());
    setTextureFilter(f);
    f->glUniform1i(f->glGetUniformLocation(m_causticsProgram->programId(), "simulationTexture"), 0);

    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(QOpenGLTexture::Target2D, m_fboNormals->texture());
    setTextureFilter(f);
    f->glUniform1i(f->glGetUniformLocation(m_causticsProgram->programId(), "normalsTexture"), 1);

    setUniforms(m_causticsProgram);

    m_fboCaustics->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fboCaustics->release();

    m_causticsProgram->release();

    if (!m_activateDemoMode)
    {
        // Render pass
        // Calculation happens at render size
        f->glViewport(0, 0, width(), height());
        m_renderProgram->bind();

        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, m_fboSimulation->texture());
        setTextureFilter(f);
        f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "simulationTexture"), 0);

        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, m_textureBaseColor->textureId());
        setTextureFilter(f);
        f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "albedoTexture"), 1);

        f->glActiveTexture(GL_TEXTURE2);
        f->glBindTexture(GL_TEXTURE_2D, m_textureEnvironmentMap->textureId());
        setTextureFilter(f);
        f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "environmentTexture"), 2);

        f->glActiveTexture(GL_TEXTURE3);
        f->glBindTexture(GL_TEXTURE_2D, m_fboNormals->texture());
        setTextureFilter(f);
        f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "normalsTexture"), 3);

        f->glActiveTexture(GL_TEXTURE4);
        f->glBindTexture(GL_TEXTURE_2D, m_fboCaustics->texture());
        setTextureFilter(f);
        f->glUniform1i(f->glGetUniformLocation(m_renderProgram->programId(), "causticsTexture"), 4);

        setUniforms(m_renderProgram);

        f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);

        m_renderProgram->release();
    } else {
        f->glViewport(0, 0, width(), height());

        m_demoProgram->bind();

        f->glActiveTexture(GL_TEXTURE0);

        switch(m_demoMode)
        {
        case NORMAL:
            f->glBindTexture(GL_TEXTURE_2D, m_fboNormals->texture());
            break;
        case SIMULATION:
            f->glBindTexture(GL_TEXTURE_2D, m_fboSimulation->texture());
            break;
        case CAUSTICS:
            f->glBindTexture(GL_TEXTURE_2D, m_fboCaustics->texture());
            break;
        }

        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        f->glUniform1i(f->glGetUniformLocation(m_demoProgram->programId(), "texture"), 0);

        setUniforms(m_demoProgram);

        f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);

        m_demoProgram->release();

        QPainter painter = QPainter();

        painter.begin(m_openGlPaintDevice);
        painter.setRenderHints(QPainter::Antialiasing);
        painter.setPen(QPen(QColor(0, 0, 0, 255), 5));
        painter.setBrush(QColor(0, 0, 0, 255));

        painter.drawRect(QRectF(0, height() - 30, 150, 30));
        painter.setPen(QPen(QColor(255, 255, 0, 255), 5));
        QFont font = painter.font();
        font.setPixelSize(18);
        painter.setFont(font);
        QString text;
        switch(m_demoMode)
        {
        case NORMAL:
            text = QString("Normal pass");
            break;
        case SIMULATION:
            text = QString("Simulation pass");
            break;
        case CAUSTICS:
            text = QString("Caustics pass");
            break;
        }

        painter.drawText(QRectF(0, height() - 30, 150, 30), text);
                painter.end();
    }

    // Draw FPS Counter
    QPainter painter = QPainter();

    painter.begin(m_openGlPaintDevice);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(0, 0, 0, 255), 5));
    painter.setBrush(QColor(0, 0, 0, 255));

    painter.drawRect(QRectF(0, 0, 60, 30));
    painter.setPen(QPen(QColor(255, 255, 0, 255), 5));
    QFont font = painter.font();
    font.setPixelSize(18);
    painter.setFont(font);
    painter.drawText(QRectF(0, 0, 60, 30), QString::number((int)(1 / u_dt)) + " FPS");
    painter.end();

    m_vao->release();
}

void GLWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_pressed) return;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT);

    f->glViewport(0, 0, m_simSize.width(), m_simSize.height());
    m_interactionProgram->bind();
    m_vao->bind();

    f->glBindTexture(QOpenGLTexture::Target2D, m_fboSimulation->texture());

    m_interactionProgram->setUniformValue("uSphere", event->position().x() / width(), 1. - event->position().y() / height(), 0.02, -0.1);
    m_fboSimulation->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fboSimulation->release();
    m_vao->release();
}

void GLWindow::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    m_lastPoint = event->position();

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT);

    f->glViewport(0, 0, m_simSize.width(), m_simSize.height());
    m_interactionProgram->bind();
    m_vao->bind();

    f->glBindTexture(QOpenGLTexture::Target2D, m_fboSimulation->texture());

    m_interactionProgram->setUniformValue("uSphere", event->position().x() / width(), 1. - event->position().y() / height(), 0.05, -0.5);
    m_fboSimulation->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, sizeof(m_quad) / 3.0f);
    m_fboSimulation->release();
    m_vao->release();
}

void GLWindow::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
}

void GLWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key::Key_D)
    {
        m_activateDemoMode = !m_activateDemoMode;
    }
    if (m_activateDemoMode)
    {
        if (key == NORMAL || key == SIMULATION || key == CAUSTICS)
        {
            m_demoMode = key;
        }
    }
}
