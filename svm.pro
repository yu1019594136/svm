HEADERS += \
    svm.h \
    svm-scale.h \
    svm-train.h \
    svm-predict.h \
    grid_search.h \
    common.h \
    aco_search.h

SOURCES += \
    svm.cpp \
    svm-scale.c \
    main.c \
    svm-train.c \
    svm-predict.c \
    grid_search.c \
    common.c \
    aco_search.c

OTHER_FILES += \
    svm.def
