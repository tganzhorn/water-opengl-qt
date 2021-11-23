#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QDebug>
#include <glwindow.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;

    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL) {
        qDebug("Requesting 4.6 OpenGL core context");
        format.setVersion(4, 6);
        format.setProfile(QSurfaceFormat::CoreProfile);
    } else {
        qDebug("Requesting 3.0 context");
        format.setVersion(3, 0);
    }

    QSurfaceFormat::setDefaultFormat(format);

    GLWindow glWindow;
    glWindow.setMinimumSize(QSize(512, 512));
    glWindow.setMaximumSize(QSize(512, 512));
    glWindow.show();

    return app.exec();
}
