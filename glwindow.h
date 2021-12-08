#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QOpenGLWindow>
#include <QDateTime>
#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QPainter>
#include <QOpenGLPaintDevice>
#include <QVector3D>

class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QMouseEvent;
class QWheelEvent;

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
    QOpenGLShaderProgram *m_causticsProgram = nullptr;
    QOpenGLShaderProgram *m_causticsComputeProgram = nullptr;
    QOpenGLShaderProgram *m_demoProgram = nullptr;
    QOpenGLBuffer *m_vbo = nullptr;
    QOpenGLVertexArrayObject *m_vao = nullptr;
    QOpenGLFramebufferObject *m_fboSimulation = nullptr;
    QOpenGLFramebufferObject *m_fboNormals = nullptr;
    QOpenGLFramebufferObject *m_fboCaustics = nullptr;
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
    float u_k = 0.8f;
    float u_damping = 0.98f;
    float u_normalStrength = 0.5f;
    float u_lightStrength = 23.0f;
    QVector3D u_lightPosition = QVector3D(0.5, 0.5, 3);
    QVector3D u_planePosition = QVector3D(0, 0, -0.1);
    QVector3D u_planeNormal = QVector3D(0, 0, 1);
    QVector3D u_cameraPosition = QVector3D(0.5, 0.5, 8);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    QSize m_simSize = QSize(128, 128);


    bool m_pressed = false;
    QPointF m_lastPoint;
    bool m_activateDemoMode = false;
    bool m_activateSimulationFreeze = false;
    int m_demoMode = Qt::Key_1;
    enum m_demoModes {
        NORMAL = Qt::Key_1,
        SIMULATION = Qt::Key_2,
        CAUSTICS = Qt::Key_3
    };

    void setUniforms(QOpenGLShaderProgram *program);
    void setTextureFilter(QOpenGLFunctions *f);
};

#endif // GLWINDOW_H
