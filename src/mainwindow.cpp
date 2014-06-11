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
#include "termwidgetholder.h"
#include "constants.h"
#include "preferences.h"
#include "preferencesdialog.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QToolButton>

MainWindow::MainWindow(const QString &workingDir, const QString &command, QWidget *parent,
                       Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_ui(new Ui::MainWindow),
    m_preferences(Preferences::instance()),
    m_actionManager(new ActionManager(this))
{
    /// TODO: Check why it is not set by default
    setAttribute(Qt::WA_DeleteOnClose);

    m_ui->setupUi(this);

    connect(m_preferences, &Preferences::changed, this, &MainWindow::preferencesChanged);

    setContentsMargins(0, 0, 0, 0);
    restoreGeometry(m_preferences->mainWindowGeometry);
    restoreState(m_preferences->mainWindowState);

    connect(m_ui->consoleTabulator, SIGNAL(closeTabNotification()), SLOT(close()));
    m_ui->consoleTabulator->setWorkDirectory(workingDir);
    m_ui->consoleTabulator->setTabPosition((QTabWidget::TabPosition)m_preferences->tabsPos);
    // ui->consoleTabulator->setShellProgram(command);
    m_ui->consoleTabulator->addNewTab(command);

    setWindowTitle("QTerminal");
    setWindowIcon(QIcon(":/icons/qterminal.png"));

    setupFileMenu();
    setupEditMenu();
    setupViewMenu();
    setupHelpMenu();
    setupContextMenu();
    setupWindowActions();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::enableDropMode()
{
    m_dropMode = true;
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    setStyleSheet(QStringLiteral("MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"));

    m_dropLockButton = new QToolButton(this);
    m_ui->consoleTabulator->setCornerWidget(m_dropLockButton, Qt::BottomRightCorner);
    m_dropLockButton->setCheckable(true);
    m_dropLockButton->connect(m_dropLockButton, SIGNAL(clicked(bool)), SLOT(setKeepOpen(bool)));
    setKeepOpen(m_preferences->dropKeepOpen);
    m_dropLockButton->setAutoRaise(true);
    realign();
}

void MainWindow::setupFileMenu()
{
    QAction *action;
    action = m_actionManager->action(ActionId::NewTab);
    connect(action, SIGNAL(triggered()), m_ui->consoleTabulator, SLOT(addNewTab()));
    addAction(action);
    m_ui->fileMenu->addAction(action);

    action = m_actionManager->action(ActionId::CloseTab);
    connect(action, SIGNAL(triggered()), m_ui->consoleTabulator, SLOT(removeCurrentTab()));
    addAction(action);
    m_ui->fileMenu->addAction(action);

    m_ui->fileMenu->addSeparator();

    action = m_actionManager->action(ActionId::NewWindow);
    connect(action, &QAction::triggered, this, &MainWindow::newWindow);
    addAction(action);
    m_ui->fileMenu->addAction(action);

    action = m_actionManager->action(ActionId::CloseWindow);
    connect(action, &QAction::triggered, this, &MainWindow::close);
    addAction(action);
    m_ui->fileMenu->addAction(action);

    m_ui->fileMenu->addSeparator();

    action = m_actionManager->action(ActionId::Exit);
    connect(action, &QAction::triggered, this, &MainWindow::quit);
    addAction(action);
    m_ui->fileMenu->addAction(action);
}

void MainWindow::setupEditMenu()
{
    QAction *action;
    action = m_actionManager->action(ActionId::Copy);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->copyClipboard();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    action = m_actionManager->action(ActionId::Paste);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteClipboard();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    action = m_actionManager->action(ActionId::PasteSelection);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteSelection();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->action(ActionId::Clear);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->clear();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->action(ActionId::Find);
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->toggleShowSearchBar();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->action(ActionId::Preferences);
    connect(action, SIGNAL(triggered()), SLOT(showPreferencesDialog()));
    addAction(action);
    m_ui->editMenu->addAction(action);
}

void MainWindow::setupViewMenu()
{
    QAction *action;
    action = m_actionManager->action(ActionId::ShowMenu);
    action->setCheckable(true);
    action->setChecked(m_preferences->menuVisible);
    connect(action, &QAction::triggered, this, &MainWindow::toggleMenuBar);
    addAction(action);
    m_ui->viewMenu->addAction(action);

    action = m_actionManager->action(ActionId::ShowTabs);
    action->setCheckable(true);
    action->setChecked(!m_preferences->tabBarless);
    connect(action, &QAction::triggered, this, &MainWindow::toggleTabBar);
    addAction(action);
    m_ui->viewMenu->addAction(action);

    m_ui->viewMenu->addSeparator();

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
            m_ui->consoleTabulator, &TabWidget::changeTabPosition);

    tabPosMenu = new QMenu(tr("Tabs Layout"), m_ui->viewMenu);
    tabPosMenu->setObjectName("tabPosMenu");

    foreach (QAction *action, tabBarPosition->actions())
        tabPosMenu->addAction(action);

    connect(m_ui->viewMenu, &QMenu::hovered, this, &MainWindow::updateActionGroup);
    m_ui->viewMenu->addMenu(tabPosMenu);

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
            m_ui->consoleTabulator, &TabWidget::changeScrollPosition);

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), m_ui->viewMenu);
    scrollPosMenu->setObjectName("scrollPosMenu");

    foreach (QAction *action, scrollBarPosition->actions())
        scrollPosMenu->addAction(action);

    m_ui->viewMenu->addMenu(scrollPosMenu);
}

void MainWindow::setupHelpMenu()
{
    QAction *action;
    action = m_actionManager->action(ActionId::About);
    connect(action, &QAction::triggered, this, &MainWindow::showAboutMessageBox);
    addAction(action);
    m_ui->helpMenu->addAction(action);

    action = m_actionManager->action(ActionId::AboutQt);
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    addAction(action);
    m_ui->helpMenu->addAction(action);
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
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::splitHorizontally);
    addAction(action);
    m_contextMenu->addAction(action);

    action = m_actionManager->action(ActionId::SplitVertically);
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::splitVertically);
    addAction(action);
    m_contextMenu->addAction(action);

    m_contextMenu->addSeparator();

    action = m_actionManager->action(ActionId::CloseTerminal);
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::splitCollapse);
    addAction(action);
    m_contextMenu->addAction(action);

    connect(m_ui->consoleTabulator->terminalHolder(),
            &TermWidgetHolder::terminalContextMenuRequested,
            [this] (const QPoint &pos) {
        m_contextMenu->exec(currentTerminal()->mapToGlobal(pos));
    });
}

void MainWindow::setupWindowActions()
{
    QAction *action;
    action = m_actionManager->action(ActionId::NextTab);
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::switchToRight);
    addAction(action);

    action = m_actionManager->action(ActionId::PreviousTab);
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::switchToLeft);
    addAction(action);
}

void MainWindow::toggleTabBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowTabs)->isChecked();
    m_ui->consoleTabulator->tabBar()->setVisible(newVisible);
    m_preferences->tabBarless = !newVisible;
}

void MainWindow::toggleMenuBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowMenu)->isChecked();
    m_ui->menuBar->setVisible(newVisible);
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
    m_ui->consoleTabulator->setTabPosition((QTabWidget::TabPosition)m_preferences->tabsPos);
    m_ui->consoleTabulator->preferencesChanged();

    m_ui->menuBar->setVisible(m_preferences->menuVisible);

    m_preferences->save();
    realign();
}

void MainWindow::realign()
{
    if (!m_dropMode)
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
    if (!m_dropLockButton)
        return;

    if (value)
        m_dropLockButton->setIcon(QIcon(":/icons/locked.png"));
    else
        m_dropLockButton->setIcon(QIcon(":/icons/notlocked.png"));

    m_dropLockButton->setChecked(value);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_preferences->askOnExit || !m_ui->consoleTabulator->count()) {
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
            && m_dropMode
            && !m_preferences->dropKeepOpen
            && qApp->activeWindow() == nullptr) {
        hide();
    }
    return QMainWindow::event(event);
}

TermWidget *MainWindow::currentTerminal() const
{
    return m_ui->consoleTabulator->terminalHolder()->currentTerminal();
}
