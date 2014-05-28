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

class QToolButton;
class QxtGlobalShortcut;

class MainWindow : public QMainWindow, private Ui::mainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &work_dir, const QString &command, bool dropMode,
                        QWidget *parent = nullptr, Qt::WindowFlags f = 0);

    bool dropMode() const;

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
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *event);

private:
    void migrate_settings();

    void setup_FileMenu_Actions();
    void setup_ActionsMenu_Actions();
    void setup_ViewMenu_Actions();

    void enableDropMode();
    void realign();

    QActionGroup *tabPosition, *scrollBarPosition;
    QMenu *tabPosMenu, *scrollPosMenu;

    QAction *toggleBorder, *toggleTabbar, *renameSession;

    QString m_initShell;
    QString m_initWorkDir;

    QDockWidget *m_bookmarksDock;

    QToolButton *m_dropLockButton = nullptr;
    bool m_dropMode;

    QxtGlobalShortcut *m_dropShortcut = nullptr;
    void setDropShortcut(QKeySequence dropShortCut);
};
#endif // MAINWINDOW_H
