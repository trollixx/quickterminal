/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QHash>
#include <QIcon>
#include <QKeySequence>
#include <QObject>

class QAction;
class QWidget;

struct ActionInfo {
    QString id;
    QString text;
    QKeySequence defaultShortcut;
    QKeySequence shortcut;
    QIcon icon;
};

class ActionManager : public QObject
{
    Q_OBJECT
public:
    explicit ActionManager(QWidget *parent = nullptr);
    ~ActionManager() override;

    QAction *action(const QString &id) const;

    static ActionManager *instance(QWidget *widget);

    static QList<ActionInfo> registry();
    static ActionInfo actionInfo(const QString &id);
    static bool registerAction(const QString &id, const ActionInfo &info);
    static bool registerAction(const QString &id, const QString &text,
                               const QKeySequence &defaultShortcut = QKeySequence(),
                               const QIcon &icon = QIcon());
    static bool registerAction(const QString &id, const QString &text, const QIcon &icon);
    static void updateShortcut(const QString &id, const QKeySequence &shortcut);

    static QString clearActionText(const QString &text);

signals:
    void changed(const QString &id);

private:
    static QHash<QString, ActionInfo> m_actionRegistry;
    static QList<ActionManager *> m_instances;

    QHash<QString, QAction *> m_actions;
};

#endif // ACTIONMANAGER_H
