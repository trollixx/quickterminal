#include "application.h"

#include "constants.h"
#include "mainwindow.h"
#include "preferences.h"

#include <QAction>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>

#include <qxtglobalshortcut.h>

Application::Application(QObject *parent) :
    QObject(parent)
{
    connect(Preferences::instance(), &Preferences::changed,
            this, &Application::preferencesChanged);

    parseOptions();
    createWindow();
}

Application::~Application()
{
    qDeleteAll(m_windows);
    Preferences::instance()->deleteLater();
}

void Application::createWindow()
{
    MainWindow *window = new MainWindow(m_workingDir, m_command, m_dropMode);

    connect(window, &MainWindow::newWindow, this, &Application::createWindow);
    connect(window, &MainWindow::quit, this, &Application::quit);
    connect(window, &MainWindow::destroyed, this, &Application::windowDeleted);

    m_windows.append(window);

    if (m_dropMode && m_windows.size() == 1) {
        m_dropShortcut = new QxtGlobalShortcut(this);
        setDropShortcut(Preferences::instance()->dropShortCut);
        connect(m_dropShortcut, &QxtGlobalShortcut::activated,
                [window]() {
            if (window->isVisible())
                window->hide();
            else
                window->show();
        });
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
    setDropShortcut(Preferences::instance()->dropShortCut);
}

void Application::windowDeleted(QObject *object)
{
    /// FIXME: Cast should be qobject_cast
    m_windows.removeAll(static_cast<MainWindow *>(object));
}

void Application::parseOptions()
{
    QCommandLineParser parser;

    QCommandLineOption dropdownOption(
    {QStringLiteral("d"), QStringLiteral("dropdown")},
                QStringLiteral("Run in 'dropdown mode' (like Yakuake or Tilda)"));
    parser.addOption(dropdownOption);

    QCommandLineOption commandOption(
    {QStringLiteral("e"), QStringLiteral("command")},
                QStringLiteral("Specify a command to execute inside the terminal"),
                QStringLiteral("COMMAND"));
    parser.addOption(commandOption);

    QCommandLineOption workingDirectoryOption(
    {QStringLiteral("w"), QStringLiteral("working-directory")},
                QStringLiteral("Set the working directory"),
                QStringLiteral("DIR"), QDir::homePath());
    parser.addOption(workingDirectoryOption);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(qApp->arguments());

    m_dropMode = parser.isSet(dropdownOption);
    m_command = parser.value(commandOption);
    m_workingDir = parser.value(workingDirectoryOption);
}

void Application::setDropShortcut(const QKeySequence &shortcut)
{
    if (!m_dropMode || m_dropShortcut->shortcut() == shortcut)
        return;

    m_dropShortcut->setShortcut(shortcut);
    qWarning("Press \"%s\" to see the terminal.", qPrintable(shortcut.toString()));
}
