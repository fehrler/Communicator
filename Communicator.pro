#-------------------------------------------------
#
# Project created by QtCreator 2017-01-26T13:47:02
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Communicator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    keithley2410.cpp \
    messlabor.cpp

HEADERS  += mainwindow.h \
    keithley2410.h \
    messlabor.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += "-std=c++11"
