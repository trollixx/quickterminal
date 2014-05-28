TEMPLATE = app

CONFIG += c++11 link_pkgconfig
QT += gui gui-private widgets
PKGCONFIG += qtermwidget5 x11

TARGET = qterminal

DEFINES += STR_VERSION=\\\"1.0\\\"

# QxtGlobalShortcut
HEADERS += $$files(src/3rdparty/*.h)
SOURCES += $$files(src/3rdparty/*.cpp)

HEADERS += $$files(src/*.h)
SOURCES += $$files(src/*.cpp)

INCLUDEPATH += src src/3rdparty

RESOURCES += src/icons.qrc
FORMS += $$files(src/forms/*.ui)

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    BINDIR = $$PREFIX/bin

    INSTALLS += target shortcut
    target.path = $$BINDIR

    DATADIR = $$PREFIX/share
    shortcut.path = $$DATADIR/applications
    shortcut.files = qterminal.desktop
}

