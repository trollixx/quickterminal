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

#include <QGridLayout>
#include <QInputDialog>
#include <QSplitter>

TermWidgetHolder::TermWidgetHolder(const QString &wdir, const QString &shell, QWidget *parent) :
    QWidget(parent),
    m_workingDir(wdir),
    m_command(shell)
{
    setFocusPolicy(Qt::NoFocus);
    QGridLayout *lay = new QGridLayout(this);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    QSplitter *s = new QSplitter(this);
    s->setFocusPolicy(Qt::NoFocus);
    TermWidget *w = newTerm();
    s->addWidget(w);
    lay->addWidget(s);

    setLayout(lay);
}

void TermWidgetHolder::setInitialFocus()
{
    QList<TermWidget *> list = findChildren<TermWidget *>();
    if (list.isEmpty())
        return;
    list.first()->setFocus(Qt::OtherFocusReason);
}

TermWidget *TermWidgetHolder::currentTerminal() const
{
    return m_currentTerm;
}

void TermWidgetHolder::switchNextSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TermWidget *> l = findChildren<TermWidget *>();
    int ix = -1;
    foreach (TermWidget *w, l) {
        ++ix;
        // qDebug() << ix << w << w->impl() << w->impl()->hasFocus() << QApplication::focusWidget();
        // qDebug() << "parent: " << w->parent();
        if (w->impl()->hasFocus())
            break;
    }

    if (ix < l.count()-1)
        l.at(ix+1)->impl()->setFocus(Qt::OtherFocusReason);
    else if (ix == l.count()-1)
        l.at(0)->impl()->setFocus(Qt::OtherFocusReason);
}

void TermWidgetHolder::switchPrevSubterminal()
{
    // TODO/FIXME: merge switchPrevSubterminal with switchNextSubterminal
    QList<TermWidget *> l = findChildren<TermWidget *>();
    int ix = -1;
    foreach (TermWidget *w, l) {
        ++ix;
        // qDebug() << ix << w << w->impl() << w->impl()->hasFocus() << QApplication::focusWidget();
        // qDebug() << "parent: " << w->parent();
        if (w->impl()->hasFocus())
            break;
    }
    // qDebug() << ix << l.at(ix)->impl() << QApplication::focusWidget() << l;

    if (ix > 0)
        l.at(ix-1)->impl()->setFocus(Qt::OtherFocusReason);
    else if (ix == 0)
        l.at(l.count()-1)->impl()->setFocus(Qt::OtherFocusReason);
}

void TermWidgetHolder::propertiesChanged()
{
    foreach (TermWidget *w, findChildren<TermWidget *>())
        w->propertiesChanged();
}

void TermWidgetHolder::splitHorizontal(TermWidget *term)
{
    split(term, Qt::Vertical);
}

void TermWidgetHolder::splitVertical(TermWidget *term)
{
    split(term, Qt::Horizontal);
}

void TermWidgetHolder::splitCollapse(TermWidget *term)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_ASSERT(parent);
    term->setParent(0);
    delete term;

    int cnt = parent->findChildren<TermWidget *>().count();
    if (cnt == 0) {
        parent->setParent(0);
        delete parent;
    }

    QList<TermWidget *> tlist = findChildren<TermWidget *>();
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

void TermWidgetHolder::split(TermWidget *term, Qt::Orientation orientation)
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
        wd = term->impl()->workingDirectory();
        if (wd.isEmpty())
            wd = m_workingDir;
    }

    TermWidget *w = newTerm(wd);
    s->insertWidget(1, w);
    s->setSizes(sizes);

    parent->insertWidget(ix, s);
    parent->setSizes(parentSizes);

    w->setFocus(Qt::OtherFocusReason);
}

TermWidget *TermWidgetHolder::newTerm(const QString &wdir, const QString &shell)
{
    QString wd(wdir);
    if (wd.isEmpty())
        wd = m_workingDir;

    QString sh(shell);
    if (shell.isEmpty())
        sh = m_command;

    TermWidget *w = new TermWidget(wd, sh, this);
    w->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(w, &TermWidget::customContextMenuRequested,
            this, &TermWidgetHolder::terminalContextMenuRequested);
    // proxy signals
    connect(w, &TermWidget::finished, this, &TermWidgetHolder::handle_finished);
    // consume signals
    connect(w, &TermWidget::termGetFocus, this, &TermWidgetHolder::setCurrentTerminal);

    return w;
}

void TermWidgetHolder::setCurrentTerminal(TermWidget *term)
{
    m_currentTerm = term;
}

void TermWidgetHolder::handle_finished()
{
    TermWidget *w = qobject_cast<TermWidget *>(sender());
    Q_ASSERT(w);
    splitCollapse(w);
}
