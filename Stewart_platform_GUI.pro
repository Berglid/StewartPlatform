android|ios|winrt {
    error( "This example is not supported for android, ios, or winrt." )
}

!include( examples/examples.pri ) {
    error( "Couldn't find the examples.pri file!" )
}



CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/serial_settings_dialog.cpp \
    src/splashscreen.cpp \
    src/stewartplatform.cpp \
    src/utilities.cpp

HEADERS += \
    include/mainwindow.h \
    include/serial_settings_dialog.h \
    include/splashscreen.h \
    include/stewartplatform.h \
    include/utilities.h

FORMS += \
    ui/mainwindow.ui \
    ui/serial_settings_dialog.ui


QT       += widgets serialport
requires(qtConfig(combobox))
requires(qtConfig(fontcombobox))
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += resources_big

# INCLUDEPATH += "C:/Users/Frode/AppData/Local/Programs/3Dconnexion/3DxWare SDK/Inc"

INCLUDEPATH += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Inc"

LIBS += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Lib/x64/siapp.lib"
LIBS += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Lib/x64/spwmath.lib"
LIBS += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Lib/x64/spwmathD.lib"
LIBS += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Lib/x64/spwmathMT.lib"
LIBS += "/home/grans/Documents/Attachment/not to be shared/3Dconnexion/3DxWare SDK/Lib/x64/spwmathMTD.lib"

# LIBS += "C:/Users/Frode/AppData/Local/Programs/3Dconnexion/3DxWare SDK/Lib/x64/siapp.lib"
# LIBS += "C:/Users/Frode/AppData/Local/Programs/3Dconnexion/3DxWare SDK/Lib/x64/spwmath.lib"
# LIBS += "C:/Users/Frode/AppData/Local/Programs/3Dconnexion/3DxWare SDK/Lib/x64/spwmath.lib"
# LIBS += "C:/Users/Frode/AppData/Local/Programs/3Dconnexion/3DxWare SDK/Lib/x64/spwmathMT.lib"


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    assets/resources.qrc

DISTFILES += \
    Notes \
    README.md

# For some reason eigen3 is installed under `/usr/include/eigen3`
# which means that you have to included as e.g. `#include <eigen3/Eigen/Dense>`.
# The two lines below solve this.
# Source: https://askubuntu.com/a/491188
unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += eigen3
