QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 static

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    glwindow.cpp \
    main.cpp

HEADERS += \
    glwindow.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    caustics.comp \
    caustics.fsh \
    demo.fsh \
    frag.fsh \
    interaction.fsh \
    normals.fsh \
    simulation.fsh \
    vert.vsh

RESOURCES += \
    resources.qrc
