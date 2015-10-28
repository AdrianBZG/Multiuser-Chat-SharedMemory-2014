TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp \
    chatroom.cpp

include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += \
    .gitignore

HEADERS += \
    chatroom.h

INCLUDEPATH = ./include

unix:!macx: LIBS += -lrt
LIBS += -lpthread -lboost_system
