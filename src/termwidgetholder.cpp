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

#include "termwidgetholder.h"

#include "preferences.h"

#include <QVBoxLayout>
#include <QInputDialog>
#include <QSplitter>

TermWidgetHolder::TermWidgetHolder(const QString &wdir, const QString &shell, QWidget *parent) :
    QWidget(parent),
    m_workingDir(wdir),
    m_command(shell)
{
    setFocusPolicy(Qt::NoFocus);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    TerminalWidget *terminal = newTerm();

    QSplitter *splitter = new QSplitter(this);
    splitter->setFocusPolicy(Qt::NoFocus);
    splitter->addWidget(terminal);

    layout->addWidget(splitter);
    setLayout(layout);
}

void TermWidgetHolder::setInitialFocus()
{
    QList<TerminalWidget *> list = findChildren<TerminalWidget *>();
    if (list.isEmpty())
        return;
    list.first()->setFocus(Qt::OtherFocusReason);
}

TerminalWidget *TermWidgetHolder::currentTerminal() const
{
    return m_currentTerm;
}

void TermWidgetHolder::switchNextSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TerminalWidget *> l = findChildren<TerminalWidget *>();
    int ix = -1;
    foreach (TerminalWidget *w, l) {
        ++ix;
        // qDebug() << ix << w << w << w->hasFocus() << QApplication::focusWidget();
        // qDebug() << "parent: " << w->parent();
        if (w->hasFocus())
            break;
    }

    if (ix < l.count()-1)
        l.at(ix+1)->setFocus(Qt::OtherFocusReason);
    else if (ix == l.count()-1)
        l.at(0)->setFocus(Qt::OtherFocusReason);
}

void TermWidgetHolder::switchPrevSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TerminalWidget *> l = findChildren<TerminalWidget *>();
    int ix = -1;
    foreach (TerminalWidget *w, l) {
        ++ix;
        // qDebug() << ix << w << w << w->hasFocus() << QApplication::focusWidget();
        // qDebug() << "parent: " << w->parent();
        if (w->hasFocus())
            break;
    }
    // qDebug() << ix << l.at(ix) << QApplication::focusWidget() << l;

    if (ix > 0)
        l.at(ix-1)->setFocus(Qt::OtherFocusReason);
    else if (ix == 0)
        l.at(l.count()-1)->setFocus(Qt::OtherFocusReason);
}

void TermWidgetHolder::propertiesChanged()
{
    foreach (TerminalWidget *w, findChildren<TerminalWidget *>())
        w->propertiesChanged();
}

void TermWidgetHolder::splitHorizontal(TerminalWidget *term)
{
    split(term, Qt::Vertical);
}

void TermWidgetHolder::splitVertical(TerminalWidget *term)
{
    split(term, Qt::Horizontal);
}

void TermWidgetHolder::splitCollapse(TerminalWidget *term)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_ASSERT(parent);
    term->setParent(0);
    delete term;

    int cnt = parent->findChildren<TerminalWidget *>().count();
    if (cnt == 0) {
        parent->setParent(0);
        delete parent;
    }

    QList<TerminalWidget *> tlist = findChildren<TerminalWidget *>();
    int localCnt = tlist.count();
    if (localCnt > 0) {
        tlist.at(0)->setFocus(Qt::OtherFocusReason);
        update();
        if (parent)
            parent->update();
    } else {
        emit finished();
    }
}

void TermWidgetHolder::split(TerminalWidget *term, Qt::Orientation orientation)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_ASSERT(parent);

    int ix = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    QList<int> sizes;
    sizes << 1 << 1;

    QSplitter *s = new QSplitter(orientation, this);
    s->setFocusPolicy(Qt::NoFocus);
    s->insertWidget(0, term);

    // wdir settings
    QString wd(m_workingDir);
    if (Preferences::instance()->useCWD) {
        wd = term->workingDirectory();
        if (wd.isEmpty())
            wd = m_workingDir;
    }

    TerminalWidget *w = newTerm(wd);
    s->insertWidget(1, w);
    s->setSizes(sizes);

    parent->insertWidget(ix, s);
    parent->setSizes(parentSizes);

    w->setFocus(Qt::OtherFocusReason);
}

TerminalWidget *TermWidgetHolder::newTerm(const QString &wdir, const QString &shell)
{
    QString wd(wdir);
    if (wd.isEmpty())
        wd = m_workingDir;

    QString sh(shell);
    if (shell.isEmpty())
        sh = m_command;

    TerminalWidget *w = new TerminalWidget(wd, sh, this);
    w->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(w, &TerminalWidget::customContextMenuRequested,
            this, &TermWidgetHolder::terminalContextMenuRequested);
    // proxy signals
    connect(w, &TerminalWidget::finished, this, &TermWidgetHolder::handle_finished);
    // consume signals
    connect(w, &TerminalWidget::focused, this, &TermWidgetHolder::setCurrentTerminal);

    return w;
}

void TermWidgetHolder::setCurrentTerminal(TerminalWidget *term)
{
    m_currentTerm = term;
}

void TermWidgetHolder::handle_finished()
{
    TerminalWidget *w = qobject_cast<TerminalWidget *>(sender());
    Q_ASSERT(w);
    splitCollapse(w);
}
