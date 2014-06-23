#include "application.h"

#include "actionmanager.h"
#include "constants.h"
#include "mainwindow.h"
#include "preferences.h"

#include <QCommandLineParser>
#include <QDir>

#include <qxtglobalshortcut.h>

Application::Application(QObject *parent) :
    QObject(parent),
    m_preferences(Preferences::instance())
{
    connect(Preferences::instance(), &Preferences::changed,
            this, &Application::preferencesChanged);

    parseOptions();
    setupActions();
    loadUserShortcuts();
    createWindow();
}

Application::~Application()
{
    qDeleteAll(m_windows);
    Preferences::instance()->save();
}

void Application::createWindow()
{
    MainWindow *window = new MainWindow(m_workingDir, m_command);

    connect(window, &MainWindow::newWindow, this, &Application::createWindow);
    connect(window, &MainWindow::quit, this, &Application::quit);
    connect(window, &MainWindow::destroyed, this, &Application::windowDeleted);

    m_windows.append(window);

    if (m_dropDownMode && m_windows.size() == 1) {
        m_dropDownShortcut = new QxtGlobalShortcut(this);
        m_dropDownShortcut->setShortcut(ActionManager::actionInfo(ActionId::ToggleVisibility).shortcut);
        connect(m_dropDownShortcut, &QxtGlobalShortcut::activated,
                [window]() {
            if (window->isVisible())
                window->hide();
            else
                window->show();
        });
        window->enableDropMode();
        if (!Preferences::instance()->dropShowOnStart)
            return;
    }

    window->show();
}

void Application::quit()
{
    qApp->quit();
}

void Application::preferencesChanged()
{
    loadUserShortcuts();
}

void Application::windowDeleted(QObject *object)
{
    /// FIXME: Cast should be qobject_cast
    m_windows.removeAll(static_cast<MainWindow *>(object));
}

void Application::parseOptions()
{
    QCommandLineParser parser;

    QCommandLineOption dropDownOption(
    {QStringLiteral("d"), QStringLiteral("dropdown")},
                QStringLiteral("Run in 'dropdown mode' (like Yakuake or Tilda)"));
    parser.addOption(dropDownOption);

    QCommandLineOption commandOption(
    {QStringLiteral("e"), QStringLiteral("command")},
                QStringLiteral("Specify a command to execute inside the terminal"),
                QStringLiteral("COMMAND"));
    parser.addOption(commandOption);

    QCommandLineOption workingDirectoryOption(
    {QStringLiteral("w"), QStringLiteral("working-directory")},
                QStringLiteral("Set the working directory"),
                QStringLiteral("DIR"), QDir::currentPath());
    parser.addOption(workingDirectoryOption);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(qApp->arguments());

    m_dropDownMode = parser.isSet(dropDownOption);
    m_command = parser.value(commandOption);
    m_workingDir = parser.value(workingDirectoryOption);
}

void Application::setupActions()
{
    // Application
    ActionManager::registerAction(ActionId::About, tr("About QTerminal"),
                                  QIcon::fromTheme(QStringLiteral("help-about")));
    ActionManager::registerAction(ActionId::AboutQt, tr("About Qt"));
    ActionManager::registerAction(ActionId::Preferences, tr("Preferences"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+P")),
                                  QIcon::fromTheme(QStringLiteral("preferences-desktop")));
    ActionManager::registerAction(ActionId::Exit, tr("E&xit"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+X")),
                                  QIcon::fromTheme(QStringLiteral("application-exit")));

    // Window
    ActionManager::registerAction(ActionId::NewWindow, tr("New &Window..."),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+N")),
                                  QIcon::fromTheme(QStringLiteral("window-new")));
    ActionManager::registerAction(ActionId::CloseWindow, tr("Close Window..."),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+Q")),
                                  QIcon::fromTheme(QStringLiteral("window-close")));
    ActionManager::registerAction(ActionId::ShowMenu, tr("Show &Menu"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+M")));
    ActionManager::registerAction(ActionId::ShowTabs, tr("Show &Tabs"));
    ActionManager::registerAction(ActionId::ToggleVisibility, tr("Toggle Visibility"),
                                  QKeySequence(QStringLiteral("F12")));

    // Tab
    ActionManager::registerAction(ActionId::NewTab, tr("New &Tab..."),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+T")),
                                  QIcon::fromTheme(QStringLiteral("tab-new")));
    ActionManager::registerAction(ActionId::CloseTab, tr("Close Tab"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+W")));
    ActionManager::registerAction(ActionId::NextTab, tr("Next Tab"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+Tab")));
    ActionManager::registerAction(ActionId::PreviousTab, tr("Previous Tab"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+Alt+Tab")));

    // Terminal
    ActionManager::registerAction(ActionId::SplitHorizontally, tr("Split &Horizontally"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+H")));
    ActionManager::registerAction(ActionId::SplitVertically, tr("Split &Vertically"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+V")));
    ActionManager::registerAction(ActionId::CloseTerminal, tr("Close"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+D")),
                                  QIcon::fromTheme(QStringLiteral("window-close")));
    ActionManager::registerAction(ActionId::Copy, tr("&Copy"),
                                  QKeySequence(QStringLiteral("Ctrl+Ins")),
                                  QIcon::fromTheme(QStringLiteral("edit-copy")));
    ActionManager::registerAction(ActionId::Paste, tr("&Paste"),
                                  QKeySequence(QStringLiteral("Shift+Ins")),
                                  QIcon::fromTheme(QStringLiteral("edit-paste")));
    ActionManager::registerAction(ActionId::PasteSelection, tr("Paste &Selection"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+Ins")),
                                  QIcon::fromTheme(QStringLiteral("edit-paste")));
    ActionManager::registerAction(ActionId::Clear, tr("C&lear"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+C")),
                                  QIcon::fromTheme(QStringLiteral("edit-clear")));
    ActionManager::registerAction(ActionId::SelectAll, tr("Select &All"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+A")),
                                  QIcon::fromTheme(QStringLiteral("edit-select-all")));
    ActionManager::registerAction(ActionId::Find, tr("&Find"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+F")),
                                  QIcon::fromTheme(QStringLiteral("edit-find")));
    ActionManager::registerAction(ActionId::ZoomIn, tr("Zoom &In"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift++")),
                                  QIcon::fromTheme(QStringLiteral("zoom-in")));
    ActionManager::registerAction(ActionId::ZoomOut, tr("Zoom &Out"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+-")),
                                  QIcon::fromTheme(QStringLiteral("zoom-out")));
    ActionManager::registerAction(ActionId::ZoomReset, tr("&Reset Zoom"),
                                  QKeySequence(QStringLiteral("Ctrl+Shift+0")),
                                  QIcon::fromTheme(QStringLiteral("zoom-original")));
}

void Application::loadUserShortcuts()
{
    foreach (const QString &id, m_preferences->shortcutActions())
        ActionManager::updateShortcut(id, m_preferences->shortcut(id));
    if (m_dropDownMode)
        m_dropDownShortcut->setShortcut(ActionManager::actionInfo(ActionId::ToggleVisibility).shortcut);
}
