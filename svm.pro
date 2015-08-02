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
    qcommon.cpp \
    display_output.cpp \
    dialog_interface_style.cpp \
    login_dialog.cpp

HEADERS  += widget.h \
    aco_search.h \
    common.h \
    grid_search.h \
    svm-predict.h \
    svm-scale.h \
    svm-train.h \
    svm.h \
    thread_data_proc.h \
    qcommon.h \
    display_output.h \
    dialog_interface_style.h \
    login_dialog.h

FORMS    += widget.ui \
    display_output.ui \
    dialog_interface_style.ui \
    login_dialog.ui

RESOURCES += \
    background_image.qrc

#RC_FILE = logo_xueyuan.rc
RC_FILE = my_icon.rc

