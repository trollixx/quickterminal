/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
** Copyright (C) 2010-2014 Petr Vanek <petr@scribus.info>
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

#include "preferences.h"

#include <QApplication>
#include <QFontDatabase>
#include <QSettings>

Preferences *Preferences::m_instance = nullptr;

Preferences *Preferences::instance()
{
    if (!m_instance)
        m_instance = new Preferences();
    return m_instance;
}

Preferences::Preferences(QObject *parent) :
    QObject(parent),
    m_settings(new QSettings(this))
{
    load();
}

/// TODO: Make it work
Preferences::~Preferences()
{
    qDebug("Preferences destructor called");
    save();
    m_instance = nullptr;
}

void Preferences::load()
{
    guiStyle = m_settings->value(QStringLiteral("guiStyle"), QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme
            = m_settings->value(QStringLiteral("colorScheme"), QStringLiteral("Linux")).toString();
    highlightCurrentTerminal
            = m_settings->value(QStringLiteral("highlightCurrentTerminal"), true).toBool();

    if (m_settings->contains(QStringLiteral("font")))
        font = qvariant_cast<QFont>(m_settings->value(QStringLiteral("font")));
    else
        font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    m_settings->beginGroup(QStringLiteral("Shortcuts"));
    foreach (const QString &key, m_settings->childKeys())
        m_shortcuts.insert(key, m_settings->value(key).toString());
    m_settings->endGroup();

    mainWindowGeometry = m_settings->value(QStringLiteral("MainWindow/geometry")).toByteArray();
    mainWindowState = m_settings->value(QStringLiteral("MainWindow/state")).toByteArray();

    historyLimited = m_settings->value(QStringLiteral("HistoryLimited"), true).toBool();
    historyLimitedTo = m_settings->value(QStringLiteral("HistoryLimitedTo"), 1000).toUInt();

    emulation
            = m_settings->value(QStringLiteral("emulation"), QStringLiteral("default")).toString();

    appOpacity = m_settings->value(QStringLiteral("MainWindow/appOpacity"), 100).toInt();
    termOpacity = m_settings->value(QStringLiteral("termOpacity"), 100).toInt();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = m_settings->value(QStringLiteral("ScrollbarPosition"), 2).toInt();
    /* default to North. I'd prefer South but North is standard (they say) */
    tabsPos = m_settings->value(QStringLiteral("TabsPosition"), 0).toInt();
    alwaysShowTabs = m_settings->value(QStringLiteral("AlwaysShowTabs"), true).toBool();
    m_motionAfterPaste = m_settings->value(QStringLiteral("MotionAfterPaste"), 0).toInt();

    /* toggles */
    tabBarless = m_settings->value(QStringLiteral("TabBarless"), false).toBool();
    menuVisible = m_settings->value(QStringLiteral("MenuVisible"), true).toBool();
    askOnExit = m_settings->value(QStringLiteral("AskOnExit"), true).toBool();
    useCWD = m_settings->value(QStringLiteral("UseCWD"), false).toBool();

    m_settings->beginGroup(QStringLiteral("DropMode"));
    dropKeepOpen = m_settings->value(QStringLiteral("KeepOpen"), false).toBool();
    dropShowOnStart = m_settings->value(QStringLiteral("ShowOnStart"), true).toBool();
    dropWidht = m_settings->value(QStringLiteral("Width"), 70).toInt();
    dropHeight = m_settings->value(QStringLiteral("Height"), 45).toInt();
    m_settings->endGroup();
}

void Preferences::save()
{
    m_settings->setValue(QStringLiteral("guiStyle"), guiStyle);
    m_settings->setValue(QStringLiteral("colorScheme"), colorScheme);
    m_settings->setValue(QStringLiteral("highlightCurrentTerminal"), highlightCurrentTerminal);
    m_settings->setValue(QStringLiteral("font"), font);

    m_settings->beginGroup(QStringLiteral("Shortcuts"));
    for (auto it = m_shortcuts.cbegin(); it != m_shortcuts.cend(); ++it)
        m_settings->setValue(it.key(), it.value());
    m_settings->endGroup();

    m_settings->setValue(QStringLiteral("MainWindow/geometry"), mainWindowGeometry);
    m_settings->setValue(QStringLiteral("MainWindow/state"), mainWindowState);

    m_settings->setValue(QStringLiteral("HistoryLimited"), historyLimited);
    m_settings->setValue(QStringLiteral("HistoryLimitedTo"), historyLimitedTo);

    m_settings->setValue(QStringLiteral("emulation"), emulation);

    m_settings->setValue(QStringLiteral("MainWindow/appOpacity"), appOpacity);
    m_settings->setValue(QStringLiteral("termOpacity"), termOpacity);
    m_settings->setValue(QStringLiteral("ScrollbarPosition"), scrollBarPos);
    m_settings->setValue(QStringLiteral("TabsPosition"), tabsPos);
    m_settings->setValue(QStringLiteral("AlwaysShowTabs"), alwaysShowTabs);
    m_settings->setValue(QStringLiteral("MotionAfterPaste"), m_motionAfterPaste);
    m_settings->setValue(QStringLiteral("TabBarless"), tabBarless);
    m_settings->setValue(QStringLiteral("MenuVisible"), menuVisible);
    m_settings->setValue(QStringLiteral("AskOnExit"), askOnExit);
    m_settings->setValue(QStringLiteral("UseCWD"), useCWD);

    m_settings->beginGroup(QStringLiteral("DropMode"));
    m_settings->setValue(QStringLiteral("KeepOpen"), dropKeepOpen);
    m_settings->setValue(QStringLiteral("ShowOnStart"), dropShowOnStart);
    m_settings->setValue(QStringLiteral("Width"), dropWidht);
    m_settings->setValue(QStringLiteral("Height"), dropHeight);
    m_settings->endGroup();

    m_settings->sync();
}

bool Preferences::hasShortcut(const QString &actionId) const
{
    return m_shortcuts.contains(actionId);
}

QKeySequence Preferences::shortcut(const QString &actionId, const QKeySequence &fallback) const
{
    return m_shortcuts.value(actionId, fallback);
}

void Preferences::setShortcut(const QString &actionId, const QKeySequence &shortcut)
{
    m_shortcuts.insert(actionId, shortcut);
}

void Preferences::removeShortcut(const QString &actionId)
{
    m_shortcuts.remove(actionId);
}

QStringList Preferences::shortcutActions() const
{
    return m_shortcuts.keys();
}

void Preferences::emitChanged()
{
    emit changed();
}
