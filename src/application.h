/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class QxtGlobalShortcut;

class MainWindow;
class Preferences;

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

signals:

public slots:
    void createWindow();
    void quit();

private slots:
    void preferencesChanged();
    void windowDeleted(QObject *object);

private:
    void parseOptions();
    void setupActions();
    void loadUserShortcuts();

    Preferences * const m_preferences = nullptr;
    QList<MainWindow *> m_windows;

    // Command line options
    QString m_command;
    bool m_dropDownMode = false;
    QString m_workingDir;

    QxtGlobalShortcut *m_dropDownShortcut = nullptr;
};

#endif // APPLICATION_H
