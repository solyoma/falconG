# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# end edited by A.S. at 2018.09.23.
# ------------------------------------------------------

HEADERS += ./albums.h \
    ./csscreator.h \
    ./deletableitems.h \
    ./config.h \
    ./crc32.h \
    ./enums.h \
    ./falcong.h \
    ./languages.h \
    ./sourcehistory.h \
    ./structEdit.h \
    ./structWriter.h \
    ./stylehandler.h \
    ./support.h \
    ./thumbnailWidget.h
SOURCES += ./albums.cpp \
    ./config.cpp \
    ./crc32.cpp \
    ./csscreator.cpp \
    ./falcong.cpp \
    ./languages.cpp \
    ./main.cpp \
    ./sourcehistory.cpp \
    ./structEdit.cpp \
    ./structwriter.cpp \
    ./stylehandler.cpp \
    ./support.cpp \
    ./thumbnailWidget.cpp

FORMS += ./falcong.ui \
    ./sourcehistory.ui
RESOURCES += falcong.qrc
QMAKE_CXXFLAGS += -std=c++17 -Wno-unused-parameter
QT += widgets webengine
