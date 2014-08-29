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

#ifndef TERMWIDGETHOLDER_H
#define TERMWIDGETHOLDER_H

#include "terminalwidget.h"

#include <QWidget>

class QSplitter;

/*! \brief TermWidget group/session manager.

This widget (one per TabWidget tab) is a "proxy" widget beetween TabWidget and
unspecified count of TermWidgets. Basically it should look like a single TermWidget
for TabWidget - with its signals and slots.

Splitting and collapsing of TermWidgets is done here.
*/
class TermWidgetHolder : public QWidget
{
    Q_OBJECT

public:
    explicit TermWidgetHolder(const QString &wdir, const QString &shell = QString(),
                              QWidget *parent = nullptr);

    void propertiesChanged();
    void setInitialFocus();

    TerminalWidget *currentTerminal() const;

public slots:
    void splitHorizontal(TerminalWidget *term);
    void splitVertical(TerminalWidget *term);
    void splitCollapse(TerminalWidget *term);
    void switchNextSubterminal();
    void switchPrevSubterminal();

signals:
    void terminalContextMenuRequested(const QPoint &pos);
    void finished();

private:
    QString m_workingDir;
    QString m_command;
    TerminalWidget *m_currentTerm = nullptr;

    void split(TerminalWidget *term, Qt::Orientation orientation);
    TerminalWidget *newTerm(const QString &wdir = QString(), const QString &shell = QString());

private slots:
    void setCurrentTerminal(TerminalWidget *term);
    void handle_finished();
};

#endif // TERMWIDGETHOLDER_H
