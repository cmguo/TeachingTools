QT += widgets quick quickwidgets qml

TEMPLATE = lib
DEFINES += TEACHINGTOOLS_LIBRARY

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    teachingtools.cpp \

HEADERS += \
    TeachingTools_global.h \
    controls.h \
    teachingtools.h \

RESOURCES += \
    res/TeachingTools.qrc

include(writing/writing.pri)
include(pagebox/pagebox.pri)
include(inkstroke/inkstroke.pri)

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
}

msvc:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS+=/Zi
    QMAKE_LFLAGS+= /INCREMENTAL:NO /Debug
    target2.files = $$OUT_PWD/release/TeachingTools.pdb
    target2.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target2
}

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


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ShowBoard/release/ -lShowBoard
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ShowBoard/debug/ -lShowBoardd
else:unix: LIBS += -L$$OUT_PWD/../ShowBoard/ -lShowBoard

INCLUDEPATH += $$PWD/../ShowBoard
DEPENDPATH += $$PWD/../ShowBoard

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/release/ -lQtComposition
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/debug/ -lQtCompositiond
else:unix: LIBS += -L$$OUT_PWD/../QtComposition/ -lQtComposition

INCLUDEPATH += $$PWD/../QtComposition
DEPENDPATH += $$PWD/../QtComposition


INCLUDEPATH += $$PWD/../QtPromise/src

INCLUDEPATH += $$PWD/../qtpromise/src/qtpromise $$PWD/../qtpromise/include
#DEPENDPATH += $$PWD/../qtpromise/src/qtpromise

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../InkCanvas/release/ -lInkCanvas
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../InkCanvas/debug/ -lInkCanvasd
else:unix: LIBS += -L$$OUT_PWD/../InkCanvas/ -lInkCanvas

INCLUDEPATH += $$PWD/../InkCanvas
DEPENDPATH += $$PWD/../InkCanvas

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtEventBus/release/ -lQtEventBus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtEventBus/debug/ -lQtEventBusd
else:unix: LIBS += -L$$OUT_PWD/../QtEventBus/ -lQtEventBus

INCLUDEPATH += $$PWD/../QtEventBus
DEPENDPATH += $$PWD/../QtEventBus
