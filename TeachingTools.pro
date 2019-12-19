QT += widgets

TEMPLATE = lib
DEFINES += TEACHINGTOOLS_LIBRARY

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

QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    buttonsheet.cpp \
    teachingtools.cpp \
    whitinggrid.cpp

HEADERS += \
    TeachingTools_global.h \
    buttonsheet.h \
    teachingtools.h \
    whitinggrid.h

RESOURCES += \
    TeachingToosRes.qrc

include(controls/controls.pri)
include(pagebox/pagebox.pri)
include(inkcanvas/inkcanvas.pri)

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
}

msvc:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS+=/Zi
    QMAKE_LFLAGS+= /INCREMENTAL:NO /Debug
    target2.files = $$OUT_PWD/release/iClassRoom.pdb
    target2.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target2
}

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ShowBoard/release/ -lShowBoard
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ShowBoard/debug/ -lShowBoardd
else:unix: LIBS += -L$$OUT_PWD/../ShowBoard/ -lShowBoard

INCLUDEPATH += $$PWD/../ShowBoard
DEPENDPATH += $$PWD/../ShowBoard

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/release/ -lQtComposition
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/debug/ -lQtCompositiond
else:unix: LIBS += -L$$OUT_PWD/../QtComposition/ -lQtCompositiond

INCLUDEPATH += $$PWD/../QtComposition
DEPENDPATH += $$PWD/../QtComposition


INCLUDEPATH += $$PWD/../QtPromise/src

INCLUDEPATH += $$PWD/../qtpromise/src/qtpromise $$PWD/../qtpromise/include
#DEPENDPATH += $$PWD/../qtpromise/src/qtpromise

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../InkCanvas/release/ -lInkCanvas
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../InkCanvas/debug/ -lInkCanvasd
else:unix: LIBS += -L$$OUT_PWD/../InkCanvas/ -lInkCanvasd

INCLUDEPATH += $$PWD/../InkCanvas
DEPENDPATH += $$PWD/../InkCanvas
