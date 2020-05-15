QT += core gui charts qml quick widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_CXXFLAGS += -fopenmp
QMAKE_CFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += \
    dropgenerator.cpp \
    main.cpp \
    mainwindow.cpp \
    worker.cpp

HEADERS += \
    dropgenerator.h \
    mainwindow.h \
    worker.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qml.qrc
