QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += MyStromSwitch.cpp main.cpp


HEADERS += MyStromSwitch.h

COPIES += configuration
configuration.files = config.ini
configuration.path = $$OUT_PWD/
configuration.base = $$PWD/

CONFIG(release, debug|release):DEFINES+=QT_NO_DEBUG_OUTPUT
