QT += widgets quick quickwidgets qml multimedia

TEMPLATE = lib
DEFINES += TEACHINGTOOLS_LIBRARY
CONFIG += plugin

CONFIG += c++14

include($$(applyCommonConfig))
include($$(applyConanPlugin))

include(../config.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
prod_xkt{
  DEFINES += ENABLE_FEEDBACK
}

prod_qxkt{
}

SOURCES += \
    teachingtoolsplugin.cpp

HEADERS += \
    TeachingTools_global.h \
    controls.h \
    teachingtoolsplugin.h

RESOURCES += \
    res/TeachingTools.qrc

DISTFILES += \
    TeachingTools.json

include(writing/writing.pri)
include(pagebox/pagebox.pri)
include(inkstroke/inkstroke.pri)

includes.files = $$PWD/*.h
win32 {
    includes.path = $$[QT_INSTALL_HEADERS]/TeachingTools
    target.path = $$[QT_INSTALL_LIBS]
}
# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target includes

exists(D:/work/tools/vcpkg/installed22) {

DEFINES += HAS_TESSERACT

win32: LIBS += -LD:/work/tools/vcpkg/installed/x64-windows/lib -ltesseract41 -lleptonica-1.80.0
#else:unix: LIBS += -L$$PWD/tesseract/lib -ltesseract41 -lleptonica-1.80.0

INCLUDEPATH += D:/work/tools/vcpkg/installed/x64-windows/include

}
