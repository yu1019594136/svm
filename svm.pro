#-------------------------------------------------
#
# Project created by QtCreator 2015-07-16T20:53:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SVM_GUI
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    svm.cpp \
    aco_search.c \
    grid_search.c \
    svm-predict.c \
    svm-scale.c \
    svm-train.c \
    thread_data_proc.cpp \
    common.c \
    qcommon.cpp

HEADERS  += widget.h \
    aco_search.h \
    common.h \
    grid_search.h \
    svm-predict.h \
    svm-scale.h \
    svm-train.h \
    svm.h \
    thread_data_proc.h \
    qcommon.h

FORMS    += widget.ui

RESOURCES += \
    background_image.qrc

RC_FILE = application_pic.rc
