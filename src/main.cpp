/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "mainwindow.h"
#include "preferences.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QSettings>
#include <QTranslator>

int main(int argc, char *argv[])
{
    setenv("TERM", "xterm", 1); // TODO/FIXME: why?

    QApplication::setApplicationName("qterminal");
    QApplication::setApplicationVersion(STR_VERSION);
    QApplication::setOrganizationDomain("qterminal.org");
    // Warning: do not change settings format. It can screw bookmarks later.
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication app(argc, argv);

    QCommandLineParser parser;

    QCommandLineOption dropdownOption(
                {QStringLiteral("d"), QStringLiteral("dropdown")},
                QStringLiteral("Run in 'dropdown mode' (like Yakuake or Tilda)"));
    parser.addOption(dropdownOption);

    QCommandLineOption commandOption(
                {QStringLiteral("e"), QStringLiteral("command")},
                QStringLiteral("Specify a command to execute inside the terminal"),
                QStringLiteral("COMMAND"));
    parser.addOption(commandOption);

    QCommandLineOption workingDirectoryOption(
                {QStringLiteral("w"), QStringLiteral("working-directory")},
                QStringLiteral("Set the working directory"),
                QStringLiteral("DIR"), QDir::homePath());
    parser.addOption(workingDirectoryOption);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    bool dropMode = parser.isSet(dropdownOption);
    QString workdir = parser.value(workingDirectoryOption);
    QString shell_command = parser.value(commandOption);

    // translations
    QString fname = QString("qterminal_%1.qm").arg(QLocale::system().name().left(2));
    QTranslator translator;
#ifdef TRANSLATIONS_DIR
    qDebug() << "TRANSLATIONS_DIR: Loading translation file" << fname << "from dir"
             << TRANSLATIONS_DIR;
    qDebug() << "load success:" << translator.load(fname, TRANSLATIONS_DIR, "_");
#endif
#ifdef APPLE_BUNDLE
    qDebug() << "APPLE_BUNDLE: Loading translator file" << fname << "from dir"
             << QApplication::applicationDirPath()+"../translations";
    qDebug() << "load success:" << translator.load(fname,
                                                   QApplication::applicationDirPath()+"../translations",
                                                   "_");
#endif
    app.installTranslator(&translator);

    MainWindow *window;
    if (dropMode) {
        QWidget *hiddenPreviewParent = new QWidget(0, Qt::Tool);
        window = new MainWindow(workdir, shell_command, dropMode, hiddenPreviewParent);
        if (Preferences::instance()->dropShowOnStart)
            window->show();
    } else {
        window = new MainWindow(workdir, shell_command, dropMode);
        window->show();
    }

    return app.exec();
}
