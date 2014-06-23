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

class ActionManager;
class Preferences;
class TermWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &workingDir, const QString &command,
                        QWidget *parent = nullptr, Qt::WindowFlags f = 0);
    ~MainWindow();

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

    Ui::MainWindow *m_ui = nullptr;

    Preferences * const m_preferences = nullptr;
    ActionManager *m_actionManager = nullptr;

    QMenu *m_contextMenu = nullptr;

    QActionGroup *tabBarPosition = nullptr;
    QActionGroup *scrollBarPosition = nullptr;
    QMenu *tabPosMenu, *scrollPosMenu;

    QToolButton *m_dropDownLockButton = nullptr;
    bool m_dropDownMode = false;
};
#endif // MAINWINDOW_H
