TARGET = qterminal
TEMPLATE = app

QT += widgets

CONFIG += link_pkgconfig
PKGCONFIG += qtermwidget5

DEFINES += STR_VERSION=\\\"1.0\\\"

SOURCES += $$files(src/*.cpp)
HEADERS += $$files(src/*.h)

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

