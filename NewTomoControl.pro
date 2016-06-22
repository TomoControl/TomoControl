#-------------------------------------------------
#
# Project created by QtCreator 2015-09-30T14:31:35
#
#-------------------------------------------------

QT       += core gui network serialport multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NewTomoControl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    alphacam.cpp \
    crc16.cpp \
    rapeltechmed.cpp \
    stepmotor_rotate.cpp \
    mygraphicsscene.cpp \
    myhistrect.cpp \
    myimageviewer.cpp \
    tiff_image.cpp \
    dialog.cpp \
    stepmotor.cpp \
    plcmwidget.cpp \
    plcm.cpp \
    mltcam.cpp


HEADERS  += mainwindow.h \
    alphacam.h \
    crc16.h \
    rapeltechmed.h \
    stepmotor_rotate.h \
    mygraphicsscene.h \
    myhistrect.h \
    myimageviewer.h \
    definitions.h \
    tiff_image.h \
    dialog.h \
    structures.h \
    stepmotor.h \
    plcmwidget.h \
    plcm.h \
    mltcam.h


FORMS    += mainwindow.ui \
    dialog.ui \
    plcmwidget.ui

RESOURCES += \
    icons.qrc

INCLUDEPATH += $$quote(C:/Program Files/Pleora Technologies Inc/eBUS SDK/Includes)
LIBS += -L$$quote(C:/Program Files/Pleora Technologies Inc/eBUS SDK/Libraries)


