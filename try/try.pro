QT += core gui
QT += multimedia multimediawidgets


greaterThan(QT_MAJOR_VERSION,4) : QT += widgets

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    my_label.cpp

HEADERS += \
    mainwindow.h \
    my_label.h

RESOURCES += \
    resources.qrc
