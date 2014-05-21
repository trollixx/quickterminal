TEMPLATE = app

CONFIG += c++11 link_pkgconfig
QT += widgets
PKGCONFIG += qtermwidget5

TARGET = qterminal

DEFINES += STR_VERSION=\\\"1.0\\\"

HEADERS += $$files(src/*.h)
SOURCES += $$files(src/*.cpp)

INCLUDEPATH += src

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

