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

#include "actionmanager.h"
#include "constants.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "termwidgetholder.h"
#include "tabwidget.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolButton>

MainWindow::MainWindow(const QString &workingDir, const QString &command, QWidget *parent,
                       Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_preferences(Preferences::instance()),
    m_actionManager(new ActionManager(this))
{
    /// TODO: Check why it is not set by default
    setAttribute(Qt::WA_DeleteOnClose);

    connect(m_preferences, &Preferences::changed, this, &MainWindow::preferencesChanged);

    m_tabWidget = new TabWidget(this);
    connect(m_tabWidget, &TabWidget::closeTabNotification, this, &MainWindow::close);

    m_tabWidget->tabBar()->setVisible(!m_preferences->tabBarless);
    m_tabWidget->setWorkDirectory(workingDir);
    m_tabWidget->setTabPosition((QTabWidget::TabPosition)m_preferences->tabsPos);
    m_tabWidget->addNewTab(command);
    setCentralWidget(m_tabWidget);

    setWindowTitle(QStringLiteral("QTerminal"));
    setWindowIcon(QIcon(QStringLiteral(":/icons/qterminal.png")));

    setupFileMenu();
    setupEditMenu();
    setupViewMenu();
    setupHelpMenu();
    setupContextMenu();
    setupWindowActions();

    setContentsMargins(0, 0, 0, 0);
    restoreGeometry(m_preferences->mainWindowGeometry);
    restoreState(m_preferences->mainWindowState);
}

MainWindow::~MainWindow()
{
}

void MainWindow::enableDropMode()
{
    m_dropDownMode = true;
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    setStyleSheet(QStringLiteral("MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"));

    m_dropDownLockButton = new QToolButton(this);
    m_tabWidget->setCornerWidget(m_dropDownLockButton, Qt::BottomRightCorner);
    m_dropDownLockButton->setCheckable(true);
    connect(m_dropDownLockButton, SIGNAL(clicked(bool)), SLOT(setKeepOpen(bool)));
    setKeepOpen(m_preferences->dropKeepOpen);
    m_dropDownLockButton->setAutoRaise(true);
    realign();
}

void MainWindow::setupFileMenu()
{
    QMenu *menu = new QMenu(tr("&File"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::NewTab);
    connect(action, SIGNAL(triggered()), m_tabWidget, SLOT(addNewTab()));
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::CloseTab);
    connect(action, SIGNAL(triggered()), m_tabWidget, SLOT(removeCurrentTab()));
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::NewWindow);
    connect(action, &QAction::triggered, this, &MainWindow::newWindow);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::CloseWindow);
    connect(action, &QAction::triggered, this, &MainWindow::close);
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Exit);
    connect(action, &QAction::triggered, this, &MainWindow::quit);
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupEditMenu()
{
    QMenu *menu = new QMenu(tr("&Edit"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::Copy);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->copyClipboard();
    });
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::Paste);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteClipboard();
    });
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::PasteSelection);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteSelection();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Clear);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->clear();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Find);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->toggleShowSearchBar();
    });
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    action = m_actionManager->action(ActionId::Preferences);
    connect(action, SIGNAL(triggered()), SLOT(showPreferencesDialog()));
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupViewMenu()
{
    QMenu *menu = new QMenu(tr("&View"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::ShowMenu);
    action->setCheckable(true);
    action->setChecked(m_preferences->menuVisible);
    connect(action, &QAction::triggered, this, &MainWindow::toggleMenuBar);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::ShowTabs);
    action->setCheckable(true);
    action->setChecked(!m_preferences->tabBarless);
    connect(action, &QAction::triggered, this, &MainWindow::toggleTabBar);
    addAction(action);
    menu->addAction(action);

    menu->addSeparator();

    // TabBar position
    tabBarPosition = new QActionGroup(this);
    tabBarPosition->addAction(tr("Top"));
    tabBarPosition->addAction(tr("Bottom"));
    tabBarPosition->addAction(tr("Left"));
    tabBarPosition->addAction(tr("Right"));

    foreach (QAction *action, tabBarPosition->actions())
        action->setCheckable(true);

    if (tabBarPosition->actions().count() > m_preferences->tabsPos)
        tabBarPosition->actions().at(m_preferences->tabsPos)->setChecked(true);

    connect(tabBarPosition, &QActionGroup::triggered,
            m_tabWidget, &TabWidget::changeTabPosition);

    tabPosMenu = new QMenu(tr("Tabs Layout"), menu);
    tabPosMenu->setObjectName(QStringLiteral("tabPosMenu"));

    foreach (QAction *action, tabBarPosition->actions())
        tabPosMenu->addAction(action);

    connect(menu, &QMenu::hovered, this, &MainWindow::updateActionGroup);
    menu->addMenu(tabPosMenu);

    // Scrollbar position
    scrollBarPosition = new QActionGroup(this);
    // Order is based on QTermWidget::ScrollBarPosition enum
    scrollBarPosition->addAction(tr("None"));
    scrollBarPosition->addAction(tr("Left"));
    scrollBarPosition->addAction(tr("Rigth"));

    foreach (QAction *action, scrollBarPosition->actions())
        action->setCheckable(true);

    if (m_preferences->scrollBarPos < scrollBarPosition->actions().size())
        scrollBarPosition->actions().at(m_preferences->scrollBarPos)->setChecked(true);

    connect(scrollBarPosition, &QActionGroup::triggered,
            m_tabWidget, &TabWidget::changeScrollPosition);

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), menu);
    scrollPosMenu->setObjectName("scrollPosMenu");

    foreach (QAction *action, scrollBarPosition->actions())
        scrollPosMenu->addAction(action);

    menu->addMenu(scrollPosMenu);

    menuBar()->addMenu(menu);
}

void MainWindow::setupHelpMenu()
{
    QMenu *menu = new QMenu(tr("&Help"), menuBar());

    QAction *action;
    action = m_actionManager->action(ActionId::About);
    connect(action, &QAction::triggered, this, &MainWindow::showAboutMessageBox);
    addAction(action);
    menu->addAction(action);

    action = m_actionManager->action(ActionId::AboutQt);
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    addAction(action);
    menu->addAction(action);

    menuBar()->addMenu(menu);
}

void MainWindow::setupContextMenu()
{
    m_contextMenu = new QMenu(this);

    m_contextMenu->addAction(m_actionManager->action(ActionId::Copy));
    m_contextMenu->addAction(m_actionManager->action(ActionId::Paste));
    m_contextMenu->addAction(m_actionManager->action(ActionId::PasteSelection));
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(m_actionManager->action(ActionId::Clear));
    m_contextMenu->addSeparator();

    QMenu *zoomMenu = new QMenu(tr("&Zoom"), m_contextMenu);

    /// TODO: Move to View Menu
    QAction *action;
    action = m_actionManager->action(ActionId::ZoomIn);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomIn();
    });
    addAction(action);
    zoomMenu->addAction(action);

    action = m_actionManager->action(ActionId::ZoomOut);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomOut();
    });
    addAction(action);
    zoomMenu->addAction(action);

    zoomMenu->addSeparator();

    action = m_actionManager->action(ActionId::ZoomReset);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomReset();
    });
    addAction(action);
    zoomMenu->addAction(action);

    m_contextMenu->addMenu(zoomMenu);

    m_contextMenu->addSeparator();

    action = m_actionManager->action(ActionId::SplitHorizontally);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitHorizontally);
    addAction(action);
    m_contextMenu->addAction(action);

    action = m_actionManager->action(ActionId::SplitVertically);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitVertically);
    addAction(action);
    m_contextMenu->addAction(action);

    m_contextMenu->addSeparator();

    action = m_actionManager->action(ActionId::CloseTerminal);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::splitCollapse);
    addAction(action);
    m_contextMenu->addAction(action);

    connect(m_tabWidget->terminalHolder(),
            &TermWidgetHolder::terminalContextMenuRequested,
            [this] (const QPoint &pos) {
        m_contextMenu->exec(currentTerminal()->mapToGlobal(pos));
    });
}

void MainWindow::setupWindowActions()
{
    QAction *action;
    action = m_actionManager->action(ActionId::NextTab);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::switchToRight);
    addAction(action);

    action = m_actionManager->action(ActionId::PreviousTab);
    connect(action, &QAction::triggered, m_tabWidget, &TabWidget::switchToLeft);
    addAction(action);
}

void MainWindow::toggleTabBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowTabs)->isChecked();
    m_tabWidget->tabBar()->setVisible(newVisible);
    m_preferences->tabBarless = !newVisible;
}

void MainWindow::toggleMenuBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowMenu)->isChecked();
    menuBar()->setVisible(newVisible);
    m_preferences->menuVisible = newVisible;
}

void MainWindow::showAboutMessageBox()
{
    QMessageBox::about(this, QString("QTerminal %1").arg(qApp->applicationVersion()),
                       tr("A lightweight multiplatform terminal emulator"));
}

void MainWindow::showPreferencesDialog()
{
    QScopedPointer<PreferencesDialog> pd(new PreferencesDialog(this));
    pd->exec();
}

void MainWindow::preferencesChanged()
{
    QApplication::setStyle(m_preferences->guiStyle);
    setWindowOpacity(m_preferences->appOpacity / 100.0);
    m_tabWidget->setTabPosition((QTabWidget::TabPosition)m_preferences->tabsPos);
    m_tabWidget->preferencesChanged();

    menuBar()->setVisible(m_preferences->menuVisible);

    m_preferences->save();
    realign();
}

void MainWindow::realign()
{
    if (!m_dropDownMode)
        return;
    QRect desktop = QApplication::desktop()->availableGeometry(this);
    QRect geometry = QRect(0, 0,
                           desktop.width()  * m_preferences->dropWidht  / 100,
                           desktop.height() * m_preferences->dropHeight / 100);
    geometry.moveCenter(desktop.center());
    // do not use 0 here - we need to calculate with potential panel on top
    geometry.setTop(desktop.top());
    setGeometry(geometry);
}

void MainWindow::updateActionGroup(QAction *a)
{
    if (a->parent()->objectName() == tabPosMenu->objectName())
        tabBarPosition->actions().at(m_preferences->tabsPos)->setChecked(true);
}

void MainWindow::showHide()
{
    if (isVisible()) {
        hide();
    } else {
        realign();
        show();
        activateWindow();
    }
}

void MainWindow::setKeepOpen(bool value)
{
    m_preferences->dropKeepOpen = value;
    if (!m_dropDownLockButton)
        return;

    if (value)
        m_dropDownLockButton->setIcon(QIcon(QStringLiteral(":/icons/locked.png")));
    else
        m_dropDownLockButton->setIcon(QIcon(QStringLiteral(":/icons/notlocked.png")));

    m_dropDownLockButton->setChecked(value);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_preferences->askOnExit || !m_tabWidget->count()) {
        m_preferences->mainWindowGeometry = saveGeometry();
        m_preferences->mainWindowState = saveState();
        event->accept();
        return;
    }

    QScopedPointer<QMessageBox> mb(new QMessageBox(this));
    mb->setWindowTitle(tr("Exit QTerminal"));
    mb->setText(tr("Are you sure you want to exit?"));
    mb->setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    QCheckBox *dontAskCheckBox = new QCheckBox(tr("Do not ask again"), mb.data());
    mb->setCheckBox(dontAskCheckBox);

    if (mb->exec() == QMessageBox::Yes) {
        m_preferences->mainWindowGeometry = saveGeometry();
        m_preferences->mainWindowState = saveState();
        m_preferences->askOnExit = !dontAskCheckBox->isChecked();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate
            && m_dropDownMode
            && !m_preferences->dropKeepOpen
            && qApp->activeWindow() == nullptr) {
        hide();
    }
    return QMainWindow::event(event);
}

TermWidget *MainWindow::currentTerminal() const
{
    return m_tabWidget->terminalHolder()->currentTerminal();
}
