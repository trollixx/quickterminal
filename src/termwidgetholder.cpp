#include "termwidgetholder.h"

#include "preferences.h"

#include <QGridLayout>
#include <QInputDialog>
#include <QSplitter>

TermWidgetHolder::TermWidgetHolder(const QString &wdir, const QString &shell, QWidget *parent) :
    QWidget(parent),
    m_wdir(wdir),
    m_shell(shell)
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

void TermWidgetHolder::loadSession()
{
    bool ok;
    QString name = QInputDialog::getItem(this, tr("Load Session"),
                                         tr("List of saved sessions:"),
                                         Preferences::instance()->sessions.keys(),
                                         0, false, &ok);
    if (!ok || name.isEmpty())
        return;
#if 0
    foreach (QWidget *w, findChildren<QWidget *>()) {
        if (w) {
            delete w;
            w = 0;
        }
    }

    qDebug() << "load" << name << QString(Preferences::instance()->sessions[name]);
    QStringList splitters = QString(Preferences::instance()->sessions[name]).split("|",
                                                                                  QString::SkipEmptyParts);
    foreach (const QString &splitter, splitters) {
        QStringList components = splitter.split(",");
        qDebug() << "comp" << components;
        // orientation
        Qt::Orientation orientation;
        if (components.size() > 0)
            orientation = components.takeAt(0).toInt();
        // sizes
        QList<int> sizes;
        QList<TermWidget *> widgets;
        foreach (const QString &s, components) {
            sizes << s.toInt();
            widgets << newTerm();
        }
        // new terms
    }
#endif
}

void TermWidgetHolder::saveSession(const QString &name)
{
    QString dump;
    QString num("%1");
    foreach (QSplitter *w, findChildren<QSplitter *>()) {
        dump += '|' + num.arg(w->orientation());
        foreach (int i, w->sizes())
            dump += ',' + num.arg(i);
    }
    Preferences::instance()->sessions[name] = dump;
}

TermWidget *TermWidgetHolder::currentTerminal() const
{
    return m_currentTerm;
}

void TermWidgetHolder::setWDir(const QString &wdir)
{
    m_wdir = wdir;
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
    QString wd(m_wdir);
    if (Preferences::instance()->useCWD) {
        wd = term->impl()->workingDirectory();
        if (wd.isEmpty())
            wd = m_wdir;
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
        wd = m_wdir;

    QString sh(shell);
    if (shell.isEmpty())
        sh = m_shell;

    TermWidget *w = new TermWidget(wd, sh, this);
    w->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(w, &TermWidget::customContextMenuRequested,
            this, &TermWidgetHolder::terminalContextMenuRequested);
    // proxy signals
    connect(w, SIGNAL(renameSession()), this, SIGNAL(renameSession()));
    connect(w, SIGNAL(removeCurrentSession()), this, SIGNAL(lastTerminalClosed()));
    connect(w, SIGNAL(finished()), this, SLOT(handle_finished()));
    // consume signals
    connect(w, SIGNAL(splitHorizontal(TermWidget *)),
            this, SLOT(splitHorizontal(TermWidget *)));
    connect(w, SIGNAL(splitVertical(TermWidget *)),
            this, SLOT(splitVertical(TermWidget *)));
    connect(w, SIGNAL(splitCollapse(TermWidget *)),
            this, SLOT(splitCollapse(TermWidget *)));
    connect(w, SIGNAL(termGetFocus(TermWidget *)),
            this, SLOT(setCurrentTerminal(TermWidget *)));

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
