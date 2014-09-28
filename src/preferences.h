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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QFont>
#include <QKeySequence>
#include <QMap>
#include <QObject>

class QAction;
class QSettings;

class Preferences : public QObject
{
    Q_OBJECT
public:
    static Preferences *instance();

    void load();
    void save();

    bool hasShortcut(const QString &actionId) const;
    QKeySequence shortcut(const QString &actionId,
                          const QKeySequence &fallback = QKeySequence()) const;
    void setShortcut(const QString &actionId, const QKeySequence &shortcut);
    void removeShortcut(const QString &actionId);
    QStringList shortcutActions() const;

    QByteArray mainWindowGeometry;
    QByteArray mainWindowState;

    QString shellCommand;

    QFont font;
    QString colorScheme;
    QString guiStyle;
    bool highlightCurrentTerminal;

    bool historyLimited;
    unsigned historyLimitedTo;

    QString emulation;

    int terminalOpacity;

    int scrollBarPosition;
    int tabBarPosition;
    bool alwaysShowTabBar;
    bool hideTabBar;

    bool menuVisible;

    int motionAfterPaste;

    bool askOnExit;

    bool useCWD;

    bool dropKeepOpen;
    bool dropShowOnStart;
    int dropWidht;
    int dropHeight;

public slots:
    /// TODO: Remove in a new implementation
    void emitChanged();

signals:
    void changed();

private:
    static Preferences *m_instance;

    Preferences(QObject *parent = nullptr);
    Q_DISABLE_COPY(Preferences)
    ~Preferences() override;

    QSettings *m_settings = nullptr;
    QMap<QString, QKeySequence> m_shortcuts;
};

#endif // PREFERENCES_H
