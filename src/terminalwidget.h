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

#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

class Preferences;

class TerminalWidget : public QTermWidget
{
    Q_OBJECT
public:
    explicit TerminalWidget(const QString &workingDir, const QString &command = QString(),
                        QWidget *parent = nullptr);

    void propertiesChanged();

    void zoomReset();

signals:
    void finished();
    void focused(TerminalWidget *self);

private:
    Preferences * const m_preferences = nullptr;
};

#endif // TERMWIDGET_H
