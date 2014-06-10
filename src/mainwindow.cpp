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
#include "config.h"
#include "constants.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "bookmarkswidget.h"

#include <QDesktopWidget>
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <QToolButton>

#include <QDebug>

// TODO/FXIME: probably remove. QSS makes it unusable on mac...
#define QSS_DROP "MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"

MainWindow::MainWindow(const QString &work_dir, const QString &command, bool dropMode,
                       QWidget *parent, Qt::WindowFlags f) :
    QMainWindow(parent, f),
    m_ui(new Ui::MainWindow),
    m_actionManager(new ActionManager(this)),
    m_initShell(command),
    m_initWorkDir(work_dir),
    m_dropMode(dropMode)
{
    /// TODO: Check why it is not set by default
    setAttribute(Qt::WA_DeleteOnClose);

    m_ui->setupUi(this);

    m_bookmarksDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarksDock->setObjectName("bookmarksDock");
    BookmarksWidget *bookmarksWidget = new BookmarksWidget(m_bookmarksDock);
    m_bookmarksDock->setWidget(bookmarksWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksDock);
    connect(bookmarksWidget, SIGNAL(callCommand(QString)),
            this, SLOT(bookmarksWidget_callCommand(QString)));

    connect(m_bookmarksDock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(bookmarksDock_visibilityChanged(bool)));

    connect(Preferences::instance(), &Preferences::changed,
            this, &MainWindow::preferencesChanged);

    setContentsMargins(0, 0, 0, 0);
    if (m_dropMode) {
        enableDropMode();
        setStyleSheet(QSS_DROP);
    } else {
        restoreGeometry(Preferences::instance()->mainWindowGeometry);
        restoreState(Preferences::instance()->mainWindowState);
    }

    connect(m_ui->consoleTabulator, SIGNAL(closeTabNotification()), SLOT(close()));
    m_ui->consoleTabulator->setWorkDirectory(work_dir);
    m_ui->consoleTabulator->setTabPosition((QTabWidget::TabPosition)Preferences::instance()->tabsPos);
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
}

void MainWindow::enableDropMode()
{
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

    m_dropLockButton = new QToolButton(this);
    m_ui->consoleTabulator->setCornerWidget(m_dropLockButton, Qt::BottomRightCorner);
    m_dropLockButton->setCheckable(true);
    m_dropLockButton->connect(m_dropLockButton, SIGNAL(clicked(bool)), SLOT(setKeepOpen(bool)));
    setKeepOpen(Preferences::instance()->dropKeepOpen);
    m_dropLockButton->setAutoRaise(true);
    realign();
}

void MainWindow::setupFileMenu()
{
    Preferences * const preferences = Preferences::instance();

    QAction *action;
    action = m_actionManager->addAction(ActionId::NewTab,
                                        tr("New &Tab..."),
                                        preferences->shortcut(ActionId::NewTab,
                                                              QStringLiteral("Ctrl+Shift+T")),
                                        QIcon::fromTheme(QStringLiteral("tab-new")));
    connect(action, SIGNAL(triggered()), m_ui->consoleTabulator, SLOT(addNewTab()));
    addAction(action);
    m_ui->fileMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::CloseTab,
                                        tr("Close Tab"),
                                        preferences->shortcut(ActionId::CloseTab,
                                                              QStringLiteral("Ctrl+Shift+W")));
    connect(action, SIGNAL(triggered()), m_ui->consoleTabulator, SLOT(removeCurrentTab()));
    addAction(action);
    m_ui->fileMenu->addAction(action);

    m_ui->fileMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::NewWindow,
                                        tr("New &Window..."),
                                        preferences->shortcut(ActionId::NewWindow,
                                                              QStringLiteral("Ctrl+Shift+N")),
                                        QIcon::fromTheme(QStringLiteral("window-new")));
    connect(action, &QAction::triggered, this, &MainWindow::newWindow);
    addAction(action);
    m_ui->fileMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::CloseWindow,
                                        tr("Close Window..."),
                                        preferences->shortcut(ActionId::CloseWindow,
                                                              QStringLiteral("Ctrl+Shift+Q")),
                                        QIcon::fromTheme(QStringLiteral("window-close")));
    connect(action, &QAction::triggered, this, &MainWindow::close);
    addAction(action);
    m_ui->fileMenu->addAction(action);

    m_ui->fileMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::Quit,
                                        tr("&Quit QTerminal"),
                                        preferences->shortcut(ActionId::Quit),
                                        QIcon::fromTheme(QStringLiteral("application-exit")));
    connect(action, &QAction::triggered, this, &MainWindow::quit);
    addAction(action);
    m_ui->fileMenu->addAction(action);
}

void MainWindow::setupEditMenu()
{
    Preferences * const preferences = Preferences::instance();

    QAction *action;
    action = m_actionManager->addAction(ActionId::Copy,
                                        tr("&Copy"),
                                        preferences->shortcut(ActionId::Copy,
                                                              QStringLiteral("Ctrl+Ins")),
                                        QIcon::fromTheme(QStringLiteral("edit-copy")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->copyClipboard();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::Paste,
                                        tr("&Paste"),
                                        preferences->shortcut(ActionId::Paste,
                                                              QStringLiteral("Shift+Ins")),
                                        QIcon::fromTheme(QStringLiteral("edit-paste")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteClipboard();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::PasteSelection,
                                        tr("Paste &Selection"),
                                        preferences->shortcut(ActionId::PasteSelection,
                                                              QStringLiteral("Ctrl+Shift+Ins")),
                                        QIcon::fromTheme(QStringLiteral("edit-paste")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->pasteSelection();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::Clear,
                                        tr("C&lear"),
                                        preferences->shortcut(ActionId::Clear,
                                                              QStringLiteral("Ctrl+Shift+X")),
                                        QIcon::fromTheme(QStringLiteral("edit-clear")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->clear();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::Find,
                                        tr("&Find"),
                                        preferences->shortcut(ActionId::Find,
                                                              QStringLiteral("Ctrl+Shift+F")),
                                        QIcon::fromTheme(QStringLiteral("edit-find")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->toggleShowSearchBar();
    });
    addAction(action);
    m_ui->editMenu->addAction(action);

    m_ui->editMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::Preferences, tr("Preferences"),
                                        preferences->shortcut(ActionId::Preferences,
                                                              QStringLiteral("Ctrl+Shift+P")),
                                        QIcon::fromTheme(QStringLiteral("preferences-desktop")));
    connect(action, SIGNAL(triggered()), SLOT(showPreferencesDialog()));
    addAction(action);
    m_ui->editMenu->addAction(action);
}

void MainWindow::setupViewMenu()
{
    Preferences * const preferences = Preferences::instance();

    QAction *action;

    toggleBorder = new QAction(tr("Hide Window Borders"), this);
    // toggleBorder->setObjectName("toggle_Borderless");
    toggleBorder->setCheckable(true);
    // TODO/FIXME: it's broken somehow. When I call toggleBorderless() here the non-responsive window appear
    // toggleBorder->setChecked(Preferences::instance()->borderless);
    // if (Preferences::instance()->borderless)
    // toggleBorderless();
    connect(toggleBorder, SIGNAL(triggered()), this, SLOT(toggleBorderless()));
    m_ui->viewMenu->addAction(toggleBorder);
    toggleBorder->setVisible(!m_dropMode);

    m_ui->viewMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::ShowMenu, tr("Show &Menu"),
                                        preferences->shortcut(ActionId::ShowMenu,
                                                              QStringLiteral("Ctrl+Shift+M")));
    action->setCheckable(true);
    action->setChecked(preferences->menuVisible);
    connect(action, &QAction::triggered, this, &MainWindow::toggleMenuBar);
    addAction(action);
    m_ui->viewMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::ShowTabs, tr("Show &Tabs"),
                                        preferences->shortcut(ActionId::ShowTabs));
    action->setCheckable(true);
    action->setChecked(!preferences->borderless);
    connect(action, &QAction::triggered, this, &MainWindow::toggleTabBar);
    addAction(action);
    m_ui->viewMenu->addAction(action);

    QSettings settings;
    settings.beginGroup("Shortcuts");
    Preferences::instance()->actions[TOGGLE_BOOKMARKS] = m_bookmarksDock->toggleViewAction();
    QKeySequence seq
            = QKeySequence::fromString(settings.value(TOGGLE_BOOKMARKS,
                                                      TOGGLE_BOOKMARKS_SHORTCUT).toString());
    Preferences::instance()->actions[TOGGLE_BOOKMARKS]->setShortcut(seq);
    m_ui->viewMenu->addAction(Preferences::instance()->actions[TOGGLE_BOOKMARKS]);
    addAction(Preferences::instance()->actions[TOGGLE_BOOKMARKS]);
    settings.endGroup();

    m_ui->viewMenu->addSeparator();

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
            m_ui->consoleTabulator, SLOT(changeTabPosition(QAction *)));

    tabPosMenu = new QMenu(tr("Tabs Layout"), m_ui->viewMenu);
    tabPosMenu->setObjectName("tabPosMenu");

    foreach (QAction *action, tabPosition->actions())
        tabPosMenu->addAction(action);

    connect(m_ui->viewMenu, SIGNAL(hovered(QAction *)),
            this, SLOT(updateActionGroup(QAction *)));
    m_ui->viewMenu->addMenu(tabPosMenu);

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
            m_ui->consoleTabulator, SLOT(changeScrollPosition(QAction *)));

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), m_ui->viewMenu);
    scrollPosMenu->setObjectName("scrollPosMenu");

    foreach (QAction *action, scrollBarPosition->actions())
        scrollPosMenu->addAction(action);

    m_ui->viewMenu->addMenu(scrollPosMenu);
}

void MainWindow::setupHelpMenu()
{
    Preferences * const preferences = Preferences::instance();

    QAction *action;
    action = m_actionManager->addAction(ActionId::About,
                                        tr("About QTerminal"),
                                        preferences->shortcut(ActionId::About),
                                        QIcon::fromTheme(QStringLiteral("help-about")));
    connect(action, &QAction::triggered, this, &MainWindow::showAboutMessageBox);
    addAction(action);
    m_ui->helpMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::AboutQt,
                                        tr("About Qt"),
                                        preferences->shortcut(ActionId::AboutQt));
    connect(action, &QAction::triggered, qApp, &QApplication::aboutQt);
    addAction(action);
    m_ui->helpMenu->addAction(action);
}

void MainWindow::setupContextMenu()
{
    Preferences * const preferences = Preferences::instance();

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
    action = m_actionManager->addAction(ActionId::ZoomIn,
                                        tr("Zoom &In"),
                                        preferences->shortcut(ActionId::ZoomIn,
                                                              QStringLiteral("Ctrl+Shift++")),
                                        QIcon::fromTheme(QStringLiteral("zoom-in")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomIn();
    });
    addAction(action);
    zoomMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::ZoomOut,
                                        tr("Zoom &Out"),
                                        preferences->shortcut(ActionId::ZoomOut,
                                                              QStringLiteral("Ctrl+Shift+-")),
                                        QIcon::fromTheme(QStringLiteral("zoom-out")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomOut();
    });
    addAction(action);
    zoomMenu->addAction(action);

    zoomMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::ZoomReset,
                                        tr("&Reset Zoom"),
                                        preferences->shortcut(ActionId::ZoomReset,
                                                              QStringLiteral("Ctrl+Shift+0")),
                                        QIcon::fromTheme(QStringLiteral("zoom-original")));
    connect(action, &QAction::triggered, [this]() {
        currentTerminal()->impl()->zoomReset();
    });
    addAction(action);
    zoomMenu->addAction(action);

    m_contextMenu->addMenu(zoomMenu);

    m_contextMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::SplitHorizontally, tr("Split &Horizontally"),
                                        preferences->shortcut(ActionId::SplitHorizontally,
                                                              QStringLiteral("Ctrl+Shift+H")));
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::splitHorizontally);
    addAction(action);
    m_contextMenu->addAction(action);

    action = m_actionManager->addAction(ActionId::SplitVertically, tr("Split &Vertically"),
                                        preferences->shortcut(ActionId::SplitVertically,
                                                              QStringLiteral("Ctrl+Shift+V")));
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::splitVertically);
    addAction(action);
    m_contextMenu->addAction(action);

    m_contextMenu->addSeparator();

    action = m_actionManager->addAction(ActionId::CloseTerminal, tr("Close"),
                                        preferences->shortcut(ActionId::CloseTerminal,
                                                              QStringLiteral("Ctrl+Shift+C")),
                                        QIcon::fromTheme(QStringLiteral("window-close")));
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
    Preferences * const preferences = Preferences::instance();

    QAction *action;
    action = m_actionManager->addAction(ActionId::NextTab,
                                        tr("Next Tab"),
                                        preferences->shortcut(ActionId::NextTab,
                                                              QStringLiteral("Ctrl+Shift+Tab")));
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::switchToRight);
    addAction(action);

    action = m_actionManager->addAction(ActionId::PreviousTab,
                                        tr("Previous Tab"),
                                        preferences->shortcut(ActionId::PreviousTab,
                                                              QStringLiteral("Ctrl+Shift+Alt+Tab")));
    connect(action, &QAction::triggered, m_ui->consoleTabulator, &TabWidget::switchToLeft);
    addAction(action);
}

void MainWindow::toggleTabBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowTabs)->isChecked();
    m_ui->consoleTabulator->tabBar()->setVisible(newVisible);
    Preferences::instance()->tabBarless = !newVisible;
}

void MainWindow::toggleBorderless()
{
    setWindowFlags(windowFlags() ^ Qt::FramelessWindowHint);
    show();
    setWindowState(Qt::WindowActive); /* don't loose focus on the window */
    Preferences::instance()->borderless = toggleBorder->isChecked();
    realign();
}

void MainWindow::toggleMenuBar()
{
    const bool newVisible = m_actionManager->action(ActionId::ShowMenu)->isChecked();
    m_ui->menuBar->setVisible(newVisible);
    Preferences::instance()->menuVisible = newVisible;
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
    QApplication::setStyle(Preferences::instance()->guiStyle);
    setWindowOpacity(Preferences::instance()->appOpacity / 100.0);
    m_ui->consoleTabulator->setTabPosition((QTabWidget::TabPosition)Preferences::instance()->tabsPos);
    m_ui->consoleTabulator->preferencesChanged();

    m_ui->menuBar->setVisible(Preferences::instance()->menuVisible);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!Preferences::instance()->askOnExit || !m_ui->consoleTabulator->count()) {
        Preferences::instance()->mainWindowGeometry = saveGeometry();
        Preferences::instance()->mainWindowState = saveState();
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
        qDebug() << "!!!";
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
    currentTerminal()->impl()->sendText(cmd);
    currentTerminal()->setFocus();
}

void MainWindow::bookmarksDock_visibilityChanged(bool visible)
{
    Preferences::instance()->bookmarksVisible = visible;
}

TermWidget *MainWindow::currentTerminal() const
{
    return m_ui->consoleTabulator->terminalHolder()->currentTerminal();
}
