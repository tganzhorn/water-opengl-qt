#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QOpenGLWindow>
#include <QDateTime>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QPainter>
#include <QOpenGLPaintDevice>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QMouseEvent;

class GLWindow : public QOpenGLWindow
{
    Q_OBJECT
public:
    GLWindow();
    ~GLWindow();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
private:
    QOpenGLShaderProgram *m_renderProgram = nullptr;
    QOpenGLShaderProgram *m_simulationProgram = nullptr;
    QOpenGLShaderProgram *m_interactionProgram = nullptr;
    QOpenGLShaderProgram *m_normalsProgram = nullptr;
    QOpenGLBuffer *m_vbo = nullptr;
    QOpenGLVertexArrayObject *m_vao = nullptr;
    QOpenGLFramebufferObject *m_fboSimulation = nullptr;
    QOpenGLFramebufferObject *m_fboNormals = nullptr;
    QOpenGLTexture *m_textureBaseColor = nullptr;
    QOpenGLTexture *m_textureEnvironmentMap = nullptr;
    QOpenGLPaintDevice *m_openGlPaintDevice = nullptr;

    float m_quad[18] = {
        -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };
    QDateTime m_time;

    float u_time = 0.0f;
    float u_dt = 0.0f;

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    QSize m_simSize = QSize(128, 128);

    bool m_pressed;
    QPointF m_lastPoint;

    void setUniforms(QOpenGLShaderProgram *program);

};

#endif // GLWINDOW_H
