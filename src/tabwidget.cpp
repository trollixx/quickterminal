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

#include "tabwidget.h"

#include "preferences.h"
#include "termwidgetholder.h"

#include <QActionGroup>
#include <QEvent>
#include <QInputDialog>
#include <QMouseEvent>
#include <QTabBar>

#define TAB_INDEX_PROPERTY "tab_index"

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);

    // On Mac OS X this will look similar to the tabs in Safari or Leopard's Terminal.app.
    setDocumentMode(true);

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);

    tabBar()->installEventFilter(this);

    connect(this, &TabWidget::tabCloseRequested, this, &TabWidget::removeTab);
}

TermWidgetHolder *TabWidget::terminalHolder() const
{
    return reinterpret_cast<TermWidgetHolder *>(widget(currentIndex()));
}

void TabWidget::setWorkDirectory(const QString &dir)
{
    m_workingDir = dir;
}

void TabWidget::addNewTab(const QString &command)
{
    const QString label = QString(tr("Shell No. %1")).arg(++m_tabNumerator);

    TermWidgetHolder *ch = terminalHolder();
    QString cwd(m_workingDir);
    if (Preferences::instance()->useCWD && ch) {
        cwd = ch->currentTerminal()->workingDirectory();
        if (cwd.isEmpty())
            cwd = m_workingDir;
    }

    TermWidgetHolder *console = new TermWidgetHolder(cwd, command, this);
    connect(console, &TermWidgetHolder::finished, this, &TabWidget::removeFinished);

    int index = addTab(console, label);
    recountIndexes();
    setCurrentIndex(index);
    console->setInitialFocus();

    showHideTabBar();
}

void TabWidget::switchNextSubterminal()
{
    terminalHolder()->switchNextSubterminal();
}

void TabWidget::switchPrevSubterminal()
{
    terminalHolder()->switchPrevSubterminal();
}

void TabWidget::splitHorizontally()
{
    terminalHolder()->splitHorizontal(terminalHolder()->currentTerminal());
}

void TabWidget::splitVertically()
{
    terminalHolder()->splitVertical(terminalHolder()->currentTerminal());
}

void TabWidget::splitCollapse()
{
    terminalHolder()->splitCollapse(terminalHolder()->currentTerminal());
}

void TabWidget::recountIndexes()
{
    for (int i = 0; i < count(); i++)
        widget(i)->setProperty(TAB_INDEX_PROPERTY, i);
}

void TabWidget::renameTab()
{
    bool ok = false;
    QString text = QInputDialog::getText(this, tr("Tab name"), tr("New tab name:"),
                                         QLineEdit::Normal, QString(), &ok);
    if (ok && !text.isEmpty())
        setTabText(currentIndex(), text);
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent *e = reinterpret_cast<QMouseEvent *>(event);
        if (tabBar()->tabAt(e->pos()) == -1)
            addNewTab();
        else
            renameTab();
        return true;
    }
    return QTabWidget::eventFilter(obj, event);
}

void TabWidget::removeFinished()
{
    QObject *term = sender();
    if (!term)
        return;
    QVariant prop = term->property(TAB_INDEX_PROPERTY);
    if (prop.isValid() && prop.canConvert(QVariant::Int)) {
        int index = prop.toInt();
        removeTab(index);
        // if (count() == 0)
        // emit closeTabNotification();
    }
}

void TabWidget::removeTab(int index)
{
    setUpdatesEnabled(false);

    QWidget *w = widget(index);
    QTabWidget::removeTab(index);
    w->deleteLater();

    recountIndexes();
    int current = currentIndex();
    if (current >= 0)
        qobject_cast<TermWidgetHolder *>(widget(current))->setInitialFocus();
    // do not decrease it as renaming is disabled in renameTabsAfterRemove
    // tabNumerator--;
    setUpdatesEnabled(true);

    if (count() == 0)
        emit lastTabClosed();

    showHideTabBar();
}

void TabWidget::removeCurrentTab()
{
    /// TODO: Add message box which can be disabled?
    removeTab(currentIndex());
}

void TabWidget::switchToRight()
{
    setCurrentIndex((currentIndex() + 1) % count());
}

void TabWidget::switchToLeft()
{
    setCurrentIndex((currentIndex() ? currentIndex() : count()) - 1);
}

void TabWidget::changeScrollPosition(QAction *triggered)
{
    QActionGroup *scrollPosition = static_cast<QActionGroup *>(sender());
    if (!scrollPosition)
        qFatal("scrollPosition is NULL");

    Preferences::instance()->scrollBarPosition = scrollPosition->actions().indexOf(triggered);

    Preferences::instance()->save();
    preferencesChanged();
}

void TabWidget::changeTabPosition(QAction *triggered)
{
    QActionGroup *tabPosition = static_cast<QActionGroup *>(sender());
    if (!tabPosition)
        qFatal("tabPosition is NULL");

    Preferences *prop = Preferences::instance();
    /* order is dictated from mainwindow.cpp */
    QTabWidget::TabPosition position
            = static_cast<QTabWidget::TabPosition>(tabPosition->actions().indexOf(triggered));
    setTabPosition(position);
    prop->tabBarPosition = position;
    prop->save();
}

void TabWidget::preferencesChanged()
{
    for (int i = 0; i < count(); ++i) {
        TermWidgetHolder *console = static_cast<TermWidgetHolder *>(widget(i));
        console->propertiesChanged();
    }
    showHideTabBar();
}

void TabWidget::showHideTabBar()
{
    tabBar()->setVisible(Preferences::instance()->alwaysShowTabBar || count() > 1);
}
