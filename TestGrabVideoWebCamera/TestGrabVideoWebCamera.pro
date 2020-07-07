QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    grabvideo.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    grabvideo.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

LIBS += -L/usr/local/lib -lavdevice -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lavfilter -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lswscale -lm -lavformat -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lavcodec -lXv -lX11 -lXext -ldl -lxcb -lxcb-shm -lX11 -lsndio -lasound -Wl,-rpath,/usr/local/lib -Wl,--enable-new-dtags -lSDL2 -lm -llzma -lbz2 -lz -pthread -lswresample -lm -lavutil -lm
LIBS += -lopencv_core \
        -lopencv_imgproc \
        -lopencv_imgcodecs \
        -lopencv_highgui \
        -lopencv_objdetect \
        -lopencv_video \
        -lopencv_videoio
