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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_qterminal.h"

#include <QMainWindow>

#ifdef LIB_QXT
#include "qxtglobalshortcut.h"
#endif

class QToolButton;

class MainWindow : public QMainWindow, private Ui::mainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &work_dir, const QString &command, bool dropMode,
               QWidget *parent = nullptr, Qt::WindowFlags f = 0);

    bool dropMode() const
    {
        return m_dropMode;
    }

protected slots:
    void on_consoleTabulator_currentChanged(int);
    void propertiesChanged();
    void actAbout_triggered();
    void actProperties_triggered();
    void updateActionGroup(QAction *);

    void toggleBorderless();
    void toggleTabBar();
    void toggleMenu();

    void showHide();
    void setKeepOpen(bool value);
    void find();

    void newTerminalWindow();
    void bookmarksWidget_callCommand(const QString &);
    void bookmarksDock_visibilityChanged(bool visible);

protected:
    bool event(QEvent *event);

private:
    QActionGroup *tabPosition, *scrollBarPosition;
    QMenu *tabPosMenu, *scrollPosMenu;

    QAction *toggleBorder, *toggleTabbar, *renameSession;

    QString m_initShell;
    QString m_initWorkDir;

    QDockWidget *m_bookmarksDock;

    void migrate_settings();

    void setup_FileMenu_Actions();
    void setup_ActionsMenu_Actions();
    void setup_ViewMenu_Actions();

    void closeEvent(QCloseEvent *event) override;

    void enableDropMode();
    QToolButton *m_dropLockButton = nullptr;
    bool m_dropMode;

    void realign();

#ifdef LIB_QXT
    QxtGlobalShortcut m_dropShortcut;
    void setDropShortcut(QKeySequence dropShortCut);
#endif
};
#endif // MAINWINDOW_H
