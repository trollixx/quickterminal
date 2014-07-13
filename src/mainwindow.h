/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
** Copyright (C) 2010-2014 Petr Vanek <petr@scribus.info>
** Copyright (C) 2006 by Vladimir Kuznetsov <vovanec@gmail.com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QActionGroup;
class QToolButton;

class ActionManager;
class Preferences;
class TabWidget;
class TermWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &workingDir, const QString &command,
                        QWidget *parent = nullptr, Qt::WindowFlags f = 0);
    ~MainWindow() override;

    void enableDropMode();

signals:
    void newWindow();
    void quit();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *event);

private slots:
    void preferencesChanged();
    void showAboutMessageBox();
    void showPreferencesDialog();
    void updateActionGroup(QAction *);

    void toggleTabBar();
    void toggleMenuBar();

    void showHide();
    void setKeepOpen(bool value);

private:
    inline TermWidget *currentTerminal() const;

    void setupFileMenu();
    void setupEditMenu();
    void setupViewMenu();
    void setupHelpMenu();
    void setupContextMenu();
    void setupWindowActions();

    void realign();

    Preferences * const m_preferences = nullptr;
    ActionManager *m_actionManager = nullptr;

    QMenu *m_contextMenu = nullptr;

    TabWidget *m_tabWidget = nullptr;
    QActionGroup *tabBarPosition = nullptr;
    QActionGroup *scrollBarPosition = nullptr;
    QMenu *tabPosMenu, *scrollPosMenu;

    QToolButton *m_dropDownLockButton = nullptr;
    bool m_dropDownMode = false;
};
#endif // MAINWINDOW_H
