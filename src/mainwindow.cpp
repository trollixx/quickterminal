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
#include "termwidgetholder.h"
#include "config.h"
#include "preferences.h"
#include "propertiesdialog.h"
#include "bookmarkswidget.h"

#include <QDesktopWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <QToolButton>

#include <qxtglobalshortcut.h>

// TODO/FXIME: probably remove. QSS makes it unusable on mac...
#define QSS_DROP    "MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"

MainWindow::MainWindow(const QString &work_dir, const QString &command, bool dropMode,
                       QWidget *parent, Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_initShell(command),
    m_initWorkDir(work_dir),
    m_dropMode(dropMode),
    m_dropShortcut(new QxtGlobalShortcut(this))
{
    setupUi(this);
    Preferences::instance()->load();

    m_bookmarksDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarksDock->setObjectName("bookmarksDock");
    BookmarksWidget *bookmarksWidget = new BookmarksWidget(m_bookmarksDock);
    m_bookmarksDock->setWidget(bookmarksWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksDock);
    connect(bookmarksWidget, SIGNAL(callCommand(QString)),
            this, SLOT(bookmarksWidget_callCommand(QString)));

    connect(m_bookmarksDock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(bookmarksDock_visibilityChanged(bool)));

    connect(actAbout, SIGNAL(triggered()), SLOT(actAbout_triggered()));
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(m_dropShortcut, SIGNAL(activated()), this, SLOT(showHide()));

    setContentsMargins(0, 0, 0, 0);
    if (m_dropMode) {
        enableDropMode();
        setStyleSheet(QSS_DROP);
    } else {
        restoreGeometry(Preferences::instance()->mainWindowGeometry);
        restoreState(Preferences::instance()->mainWindowState);
    }

    connect(consoleTabulator, SIGNAL(closeTabNotification()), SLOT(close()));
    consoleTabulator->setWorkDirectory(work_dir);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Preferences::instance()->tabsPos);
    // consoleTabulator->setShellProgram(command);
    consoleTabulator->addNewTab(command);

    setWindowTitle("QTerminal");
    setWindowIcon(QIcon(":/icons/qterminal.png"));

    setup_FileMenu_Actions();
    setup_ActionsMenu_Actions();
    setup_ViewMenu_Actions();

    // Add global rename Session shortcut
    renameSession = new QAction(tr("Rename Session"), this);
    renameSession->setShortcut(QKeySequence(tr(RENAME_SESSION_SHORTCUT)));
    connect(renameSession, SIGNAL(triggered()), consoleTabulator, SLOT(renameSession()));
    addAction(renameSession);
}

bool MainWindow::dropMode() const
{
    return m_dropMode;
}

void MainWindow::enableDropMode()
{
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

    m_dropLockButton = new QToolButton(this);
    consoleTabulator->setCornerWidget(m_dropLockButton, Qt::BottomRightCorner);
    m_dropLockButton->setCheckable(true);
    m_dropLockButton->connect(m_dropLockButton, SIGNAL(clicked(bool)), SLOT(setKeepOpen(bool)));
    setKeepOpen(Preferences::instance()->dropKeepOpen);
    m_dropLockButton->setAutoRaise(true);

    setDropShortcut(Preferences::instance()->dropShortCut);
    realign();
}

void MainWindow::setDropShortcut(QKeySequence dropShortCut)
{
    if (!m_dropMode || m_dropShortcut->shortcut() == dropShortCut)
        return;

    m_dropShortcut->setShortcut(dropShortCut);
    qWarning("Press \"%s\" to see the terminal.", qPrintable(dropShortCut.toString()));
}

void MainWindow::setup_ActionsMenu_Actions()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QKeySequence seq;

    Preferences::instance()->actions[CLEAR_TERMINAL] = new QAction(tr("Clear Current Tab"), this);
    seq = QKeySequence::fromString(settings.value(CLEAR_TERMINAL,
                                                  CLEAR_TERMINAL_SHORTCUT).toString());
    Preferences::instance()->actions[CLEAR_TERMINAL]->setShortcut(seq);
    connect(Preferences::instance()->actions[CLEAR_TERMINAL], SIGNAL(
                triggered()), consoleTabulator, SLOT(clearActiveTerminal()));
    menu_Actions->addAction(Preferences::instance()->actions[CLEAR_TERMINAL]);
    addAction(Preferences::instance()->actions[CLEAR_TERMINAL]);

    menu_Actions->addSeparator();

    Preferences::instance()->actions[TAB_NEXT] = new QAction(tr("Next Tab"), this);
    seq = QKeySequence::fromString(settings.value(TAB_NEXT, TAB_NEXT_SHORTCUT).toString());
    Preferences::instance()->actions[TAB_NEXT]->setShortcut(seq);
    connect(Preferences::instance()->actions[TAB_NEXT], SIGNAL(triggered()), consoleTabulator,
            SLOT(switchToRight()));
    menu_Actions->addAction(Preferences::instance()->actions[TAB_NEXT]);
    addAction(Preferences::instance()->actions[TAB_NEXT]);

    Preferences::instance()->actions[TAB_PREV] = new QAction(tr("Previous Tab"), this);
    seq = QKeySequence::fromString(settings.value(TAB_PREV, TAB_PREV_SHORTCUT).toString());
    Preferences::instance()->actions[TAB_PREV]->setShortcut(seq);
    connect(Preferences::instance()->actions[TAB_PREV], SIGNAL(triggered()), consoleTabulator,
            SLOT(switchToLeft()));
    menu_Actions->addAction(Preferences::instance()->actions[TAB_PREV]);
    addAction(Preferences::instance()->actions[TAB_PREV]);

    Preferences::instance()->actions[MOVE_LEFT] = new QAction(tr("Move Tab Left"), this);
    seq = QKeySequence::fromString(settings.value(MOVE_LEFT, MOVE_LEFT_SHORTCUT).toString());
    Preferences::instance()->actions[MOVE_LEFT]->setShortcut(seq);
    connect(Preferences::instance()->actions[MOVE_LEFT], SIGNAL(triggered()), consoleTabulator,
            SLOT(moveLeft()));
    menu_Actions->addAction(Preferences::instance()->actions[MOVE_LEFT]);
    addAction(Preferences::instance()->actions[MOVE_LEFT]);

    Preferences::instance()->actions[MOVE_RIGHT] = new QAction(tr("Move Tab Right"), this);
    seq = QKeySequence::fromString(settings.value(MOVE_RIGHT, MOVE_RIGHT_SHORTCUT).toString());
    Preferences::instance()->actions[MOVE_RIGHT]->setShortcut(seq);
    connect(Preferences::instance()->actions[MOVE_RIGHT], SIGNAL(
                triggered()), consoleTabulator, SLOT(moveRight()));
    menu_Actions->addAction(Preferences::instance()->actions[MOVE_RIGHT]);
    addAction(Preferences::instance()->actions[MOVE_RIGHT]);

    menu_Actions->addSeparator();

    Preferences::instance()->actions[SPLIT_HORIZONTAL] = new QAction(tr(
                                                                        "Split Terminal Horizontally"),
                                                                    this);
    seq = QKeySequence::fromString(settings.value(SPLIT_HORIZONTAL).toString());
    Preferences::instance()->actions[SPLIT_HORIZONTAL]->setShortcut(seq);
    connect(Preferences::instance()->actions[SPLIT_HORIZONTAL], SIGNAL(
                triggered()), consoleTabulator, SLOT(splitHorizontally()));
    menu_Actions->addAction(Preferences::instance()->actions[SPLIT_HORIZONTAL]);
    addAction(Preferences::instance()->actions[SPLIT_HORIZONTAL]);

    Preferences::instance()->actions[SPLIT_VERTICAL] = new QAction(tr(
                                                                      "Split Terminal Vertically"),
                                                                  this);
    seq = QKeySequence::fromString(settings.value(SPLIT_VERTICAL).toString());
    Preferences::instance()->actions[SPLIT_VERTICAL]->setShortcut(seq);
    connect(Preferences::instance()->actions[SPLIT_VERTICAL], SIGNAL(
                triggered()), consoleTabulator, SLOT(splitVertically()));
    menu_Actions->addAction(Preferences::instance()->actions[SPLIT_VERTICAL]);
    addAction(Preferences::instance()->actions[SPLIT_VERTICAL]);

    Preferences::instance()->actions[SUB_COLLAPSE] = new QAction(tr("Collapse Subterminal"), this);
    seq = QKeySequence::fromString(settings.value(SUB_COLLAPSE).toString());
    Preferences::instance()->actions[SUB_COLLAPSE]->setShortcut(seq);
    connect(Preferences::instance()->actions[SUB_COLLAPSE], SIGNAL(
                triggered()), consoleTabulator, SLOT(splitCollapse()));
    menu_Actions->addAction(Preferences::instance()->actions[SUB_COLLAPSE]);
    addAction(Preferences::instance()->actions[SUB_COLLAPSE]);

    Preferences::instance()->actions[SUB_NEXT] = new QAction(tr("Next Subterminal"), this);
    seq = QKeySequence::fromString(settings.value(SUB_NEXT, SUB_NEXT_SHORTCUT).toString());
    Preferences::instance()->actions[SUB_NEXT]->setShortcut(seq);
    connect(Preferences::instance()->actions[SUB_NEXT], SIGNAL(triggered()), consoleTabulator,
            SLOT(switchNextSubterminal()));
    menu_Actions->addAction(Preferences::instance()->actions[SUB_NEXT]);
    addAction(Preferences::instance()->actions[SUB_NEXT]);

    Preferences::instance()->actions[SUB_PREV] = new QAction(tr("Previous Subterminal"), this);
    seq = QKeySequence::fromString(settings.value(SUB_PREV, SUB_PREV_SHORTCUT).toString());
    Preferences::instance()->actions[SUB_PREV]->setShortcut(seq);
    connect(Preferences::instance()->actions[SUB_PREV], SIGNAL(triggered()), consoleTabulator,
            SLOT(switchPrevSubterminal()));
    menu_Actions->addAction(Preferences::instance()->actions[SUB_PREV]);
    addAction(Preferences::instance()->actions[SUB_PREV]);

    menu_Actions->addSeparator();

    // Copy and Paste are only added to the table for the sake of bindings at the moment; there is no Edit menu, only a context menu.
    Preferences::instance()->actions[COPY_SELECTION] = new QAction(tr("Copy Selection"), this);
    seq = QKeySequence::fromString(settings.value(COPY_SELECTION,
                                                  COPY_SELECTION_SHORTCUT).toString());
    Preferences::instance()->actions[COPY_SELECTION]->setShortcut(seq);

    Preferences::instance()->actions[PASTE_CLIPBOARD] = new QAction(tr("Paste Clipboard"), this);
    seq = QKeySequence::fromString(settings.value(PASTE_CLIPBOARD,
                                                  PASTE_CLIPBOARD_SHORTCUT).toString());
    Preferences::instance()->actions[PASTE_CLIPBOARD]->setShortcut(seq);

    Preferences::instance()->actions[PASTE_SELECTION] = new QAction(tr("Paste Selection"), this);
    seq = QKeySequence::fromString(settings.value(PASTE_SELECTION,
                                                  PASTE_SELECTION_SHORTCUT).toString());
    Preferences::instance()->actions[PASTE_SELECTION]->setShortcut(seq);

    Preferences::instance()->actions[ZOOM_IN] = new QAction(tr("Zoom in"), this);
    seq = QKeySequence::fromString(settings.value(ZOOM_IN, ZOOM_IN_SHORTCUT).toString());
    Preferences::instance()->actions[ZOOM_IN]->setShortcut(seq);

    Preferences::instance()->actions[ZOOM_OUT] = new QAction(tr("Zoom out"), this);
    seq = QKeySequence::fromString(settings.value(ZOOM_OUT, ZOOM_OUT_SHORTCUT).toString());
    Preferences::instance()->actions[ZOOM_OUT]->setShortcut(seq);

    Preferences::instance()->actions[ZOOM_RESET] = new QAction(tr("Zoom reset"), this);
    seq = QKeySequence::fromString(settings.value(ZOOM_RESET, ZOOM_RESET_SHORTCUT).toString());
    Preferences::instance()->actions[ZOOM_RESET]->setShortcut(seq);

    menu_Actions->addSeparator();

    Preferences::instance()->actions[FIND] = new QAction(tr("Find..."), this);
    seq = QKeySequence::fromString(settings.value(FIND, FIND_SHORTCUT).toString());
    Preferences::instance()->actions[FIND]->setShortcut(seq);
    connect(Preferences::instance()->actions[FIND], SIGNAL(triggered()), this, SLOT(find()));
    menu_Actions->addAction(Preferences::instance()->actions[FIND]);
    addAction(Preferences::instance()->actions[FIND]);

#if 0
    act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

    // TODO/FIXME: unimplemented for now
    act = new QAction(tr("Save Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    // act->setShortcut(QKeySequence::Save);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(saveSession()));
    addAction(act);

    act = new QAction(tr("Load Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    // act->setShortcut(QKeySequence::Open);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(loadSession()));
    addAction(act);
#endif

    Preferences::instance()->actions[TOGGLE_MENU] = new QAction(tr("Toggle Menu"), this);
    seq = QKeySequence::fromString(settings.value(TOGGLE_MENU, TOGGLE_MENU_SHORTCUT).toString());
    Preferences::instance()->actions[TOGGLE_MENU]->setShortcut(seq);
    connect(Preferences::instance()->actions[TOGGLE_MENU], SIGNAL(triggered()), this,
            SLOT(toggleMenu()));
    // tis is correct - add action to main window - not to menu to keep toggle working
    addAction(Preferences::instance()->actions[TOGGLE_MENU]);

    settings.endGroup();

    // apply props
    propertiesChanged();
}

void MainWindow::setup_FileMenu_Actions()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QKeySequence seq;

    Preferences::instance()->actions[ADD_TAB]
            = new QAction(QIcon(":/icons/list-add.png"), tr("New Tab"), this);
    seq = QKeySequence::fromString(settings.value(ADD_TAB, ADD_TAB_SHORTCUT).toString());
    Preferences::instance()->actions[ADD_TAB]->setShortcut(seq);
    connect(Preferences::instance()->actions[ADD_TAB], SIGNAL(triggered()), consoleTabulator,
            SLOT(addNewTab()));
    menu_File->addAction(Preferences::instance()->actions[ADD_TAB]);
    addAction(Preferences::instance()->actions[ADD_TAB]);

    Preferences::instance()->actions[CLOSE_TAB]
            = new QAction(QIcon(":/icons/list-remove.png"), tr("Close Tab"), this);
    seq = QKeySequence::fromString(settings.value(CLOSE_TAB, CLOSE_TAB_SHORTCUT).toString());
    Preferences::instance()->actions[CLOSE_TAB]->setShortcut(seq);
    connect(Preferences::instance()->actions[CLOSE_TAB], SIGNAL(triggered()), consoleTabulator,
            SLOT(removeCurrentTab()));
    menu_File->addAction(Preferences::instance()->actions[CLOSE_TAB]);
    addAction(Preferences::instance()->actions[CLOSE_TAB]);

    Preferences::instance()->actions[NEW_WINDOW] = new QAction(tr("New Window"), this);
    seq = QKeySequence::fromString(settings.value(NEW_WINDOW, NEW_WINDOW_SHORTCUT).toString());
    Preferences::instance()->actions[NEW_WINDOW]->setShortcut(seq);
    connect(Preferences::instance()->actions[NEW_WINDOW], SIGNAL(triggered()), this,
            SLOT(newTerminalWindow()));
    menu_File->addAction(Preferences::instance()->actions[NEW_WINDOW]);
    addAction(Preferences::instance()->actions[NEW_WINDOW]);

    menu_File->addSeparator();

    Preferences::instance()->actions[PREFERENCES] = actProperties;
    connect(actProperties, SIGNAL(triggered()), SLOT(actProperties_triggered()));
    menu_File->addAction(Preferences::instance()->actions[PREFERENCES]);

    menu_File->addSeparator();

    Preferences::instance()->actions[QUIT] = actQuit;
    connect(actQuit, SIGNAL(triggered()), SLOT(close()));
    menu_File->addAction(Preferences::instance()->actions[QUIT]);

    settings.endGroup();
}

void MainWindow::setup_ViewMenu_Actions()
{
    toggleBorder = new QAction(tr("Hide Window Borders"), this);
    // toggleBorder->setObjectName("toggle_Borderless");
    toggleBorder->setCheckable(true);
    // TODO/FIXME: it's broken somehow. When I call toggleBorderless() here the non-responsive window appear
    // toggleBorder->setChecked(Properties::Instance()->borderless);
    // if (Properties::Instance()->borderless)
    // toggleBorderless();
    connect(toggleBorder, SIGNAL(triggered()), this, SLOT(toggleBorderless()));
    menu_View->addAction(toggleBorder);
    toggleBorder->setVisible(!m_dropMode);

    toggleTabbar = new QAction(tr("Show Tab Bar"), this);
    // toggleTabbar->setObjectName("toggle_TabBar");
    toggleTabbar->setCheckable(true);
    toggleTabbar->setChecked(!Preferences::instance()->tabBarless);
    toggleTabBar();
    connect(toggleTabbar, SIGNAL(triggered()), this, SLOT(toggleTabBar()));
    menu_View->addAction(toggleTabbar);

    QSettings settings;
    settings.beginGroup("Shortcuts");
    Preferences::instance()->actions[TOGGLE_BOOKMARKS] = m_bookmarksDock->toggleViewAction();
    QKeySequence seq
            = QKeySequence::fromString(settings.value(TOGGLE_BOOKMARKS,
                                                      TOGGLE_BOOKMARKS_SHORTCUT).toString());
    Preferences::instance()->actions[TOGGLE_BOOKMARKS]->setShortcut(seq);
    menu_View->addAction(Preferences::instance()->actions[TOGGLE_BOOKMARKS]);
    addAction(Preferences::instance()->actions[TOGGLE_BOOKMARKS]);
    settings.endGroup();

    menu_View->addSeparator();

    /* tabs position */
    tabPosition = new QActionGroup(this);
    QAction *tabBottom = new QAction(tr("Bottom"), this);
    QAction *tabTop = new QAction(tr("Top"), this);
    QAction *tabRight = new QAction(tr("Right"), this);
    QAction *tabLeft = new QAction(tr("Left"), this);
    tabPosition->addAction(tabTop);
    tabPosition->addAction(tabBottom);
    tabPosition->addAction(tabLeft);
    tabPosition->addAction(tabRight);

    foreach (QAction *action, tabPosition->actions())
        action->setCheckable(true);

    if (tabPosition->actions().count() > Preferences::instance()->tabsPos)
        tabPosition->actions().at(Preferences::instance()->tabsPos)->setChecked(true);

    connect(tabPosition, SIGNAL(triggered(QAction *)),
            consoleTabulator, SLOT(changeTabPosition(QAction *)));

    tabPosMenu = new QMenu(tr("Tabs Layout"), menu_View);
    tabPosMenu->setObjectName("tabPosMenu");

    foreach (QAction *action, tabPosition->actions())
        tabPosMenu->addAction(action);

    connect(menu_View, SIGNAL(hovered(QAction *)),
            this, SLOT(updateActionGroup(QAction *)));
    menu_View->addMenu(tabPosMenu);
    /* */

    /* Scrollbar */
    scrollBarPosition = new QActionGroup(this);
    QAction *scrollNone = new QAction(tr("None"), this);
    QAction *scrollRight = new QAction(tr("Right"), this);
    QAction *scrollLeft = new QAction(tr("Left"), this);

    /* order of insertion is dep. on QTermWidget::ScrollBarPosition enum */
    scrollBarPosition->addAction(scrollNone);
    scrollBarPosition->addAction(scrollLeft);
    scrollBarPosition->addAction(scrollRight);

    foreach (QAction *action, scrollBarPosition->actions())
        action->setCheckable(true);

    if (Preferences::instance()->scrollBarPos < scrollBarPosition->actions().size())
        scrollBarPosition->actions().at(Preferences::instance()->scrollBarPos)->setChecked(true);

    connect(scrollBarPosition, SIGNAL(triggered(QAction *)),
            consoleTabulator, SLOT(changeScrollPosition(QAction *)));

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), menu_View);
    scrollPosMenu->setObjectName("scrollPosMenu");

    foreach (QAction *action, scrollBarPosition->actions())
        scrollPosMenu->addAction(action);

    menu_View->addMenu(scrollPosMenu);
}

void MainWindow::on_consoleTabulator_currentChanged(int)
{
}

void MainWindow::toggleTabBar()
{
    consoleTabulator->tabBar()->setVisible(toggleTabbar->isChecked());
    Preferences::instance()->tabBarless = !toggleTabbar->isChecked();
}

void MainWindow::toggleBorderless()
{
    setWindowFlags(windowFlags() ^ Qt::FramelessWindowHint);
    show();
    setWindowState(Qt::WindowActive); /* don't loose focus on the window */
    Preferences::instance()->borderless = toggleBorder->isChecked();
    realign();
}

void MainWindow::toggleMenu()
{
    m_menuBar->setVisible(!m_menuBar->isVisible());
    Preferences::instance()->menuVisible = m_menuBar->isVisible();
}

void MainWindow::actAbout_triggered()
{
    QMessageBox::about(this, QString("QTerminal ") + STR_VERSION,
                       tr("A lightweight multiplatform terminal emulator"));
}

void MainWindow::actProperties_triggered()
{
    QScopedPointer<PropertiesDialog> pd(new PropertiesDialog(this));
    connect(pd.data(), SIGNAL(propertiesChanged()), this, SLOT(propertiesChanged()));
    pd->exec();
}

void MainWindow::propertiesChanged()
{
    QApplication::setStyle(Preferences::instance()->guiStyle);
    setWindowOpacity(Preferences::instance()->appOpacity/100.0);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Preferences::instance()->tabsPos);
    consoleTabulator->propertiesChanged();
    setDropShortcut(Preferences::instance()->dropShortCut);

    m_menuBar->setVisible(Preferences::instance()->menuVisible);
    m_bookmarksDock->setVisible(Preferences::instance()->useBookmarks
                                && Preferences::instance()->bookmarksVisible);
    m_bookmarksDock->toggleViewAction()->setVisible(Preferences::instance()->useBookmarks);

    if (Preferences::instance()->useBookmarks)
        qobject_cast<BookmarksWidget *>(m_bookmarksDock->widget())->setup();

    Preferences::instance()->save();
    realign();
}

void MainWindow::realign()
{
    if (!m_dropMode)
        return;
    QRect desktop = QApplication::desktop()->availableGeometry(this);
    QRect geometry = QRect(0, 0,
                           desktop.width()  * Preferences::instance()->dropWidht  / 100,
                           desktop.height() * Preferences::instance()->dropHeight / 100);
    geometry.moveCenter(desktop.center());
    // do not use 0 here - we need to calculate with potential panel on top
    geometry.setTop(desktop.top());
    setGeometry(geometry);
}

void MainWindow::updateActionGroup(QAction *a)
{
    if (a->parent()->objectName() == tabPosMenu->objectName())
        tabPosition->actions().at(Preferences::instance()->tabsPos)->setChecked(true);
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
    Preferences::instance()->dropKeepOpen = value;
    if (!m_dropLockButton)
        return;

    if (value)
        m_dropLockButton->setIcon(QIcon(":/icons/locked.png"));
    else
        m_dropLockButton->setIcon(QIcon(":/icons/notlocked.png"));

    m_dropLockButton->setChecked(value);
}

void MainWindow::find()
{
    // A bit ugly perhaps with 4 levels of indirection...
    consoleTabulator->terminalHolder()->currentTerminal()->impl()->toggleShowSearchBar();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!Preferences::instance()->askOnExit || !consoleTabulator->count()) {
        Preferences::instance()->mainWindowGeometry = saveGeometry();
        Preferences::instance()->mainWindowState = saveState();
        Preferences::instance()->save();
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
        Preferences::instance()->mainWindowGeometry = saveGeometry();
        Preferences::instance()->mainWindowState = saveState();
        Preferences::instance()->askOnExit = !dontAskCheckBox->isChecked();
        Preferences::instance()->save();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate
            && m_dropMode
            && !Preferences::instance()->dropKeepOpen
            && qApp->activeWindow() == nullptr) {
        hide();
    }
    return QMainWindow::event(event);
}

void MainWindow::newTerminalWindow()
{
    MainWindow *w = new MainWindow(m_initWorkDir, m_initShell, false);
    w->show();
}

void MainWindow::bookmarksWidget_callCommand(const QString &cmd)
{
    consoleTabulator->terminalHolder()->currentTerminal()->impl()->sendText(cmd);
    consoleTabulator->terminalHolder()->currentTerminal()->setFocus();
}

void MainWindow::bookmarksDock_visibilityChanged(bool visible)
{
    Preferences::instance()->bookmarksVisible = visible;
}
