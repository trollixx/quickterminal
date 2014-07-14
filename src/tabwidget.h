/****************************************************************************
**
** Copyright (C) 2014 Oleg Shparber <trollixx+quickterminal@gmail.com>
** Copyright (C) 2010-2014 Petr Vanek <petr@scribus.info>
** Copyright (C) 2006 by Vladimir Kuznetsov <vovanec@gmail.com>
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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

class QAction;

class TermWidgetHolder;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = nullptr);

    TermWidgetHolder *terminalHolder() const;

public slots:
    int addNewTab(const QString &command = QString());
    void removeTab(int);
    void removeCurrentTab();
    int switchToRight();
    int switchToLeft();
    void removeFinished();
    void renameTab();
    void setWorkDirectory(const QString &);

    void switchNextSubterminal();
    void switchPrevSubterminal();
    void splitHorizontally();
    void splitVertically();
    void splitCollapse();

    void changeTabPosition(QAction *);
    void changeScrollPosition(QAction *);
    void preferencesChanged();

signals:
    void lastTabClosed();

protected:
    void recountIndexes();
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void showHideTabBar();

    int m_tabNumerator = 0;
    QString m_workingDir;
};

#endif // TABWIDGET_H
