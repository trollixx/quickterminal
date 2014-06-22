#include "termwidget.h"

#include "preferences.h"

#include <QDebug>
#include <QDesktopServices>
#include <QPainter>
#include <QVBoxLayout>

#define FLOW_CONTROL_ENABLED false
#define FLOW_CONTROL_WARNING_ENABLED false

static int TermWidgetCount = 0;

TermWidgetImpl::TermWidgetImpl(const QString &wdir, const QString &shell, QWidget *parent) :
    QTermWidget(0, parent)
{
    TermWidgetCount++;
    QString name("TermWidget_%1");
    setObjectName(name.arg(TermWidgetCount));

    setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);

    propertiesChanged();

    setHistorySize(5000);

    if (!wdir.isNull())
        setWorkingDirectory(wdir);

    if (shell.isNull()) {
        if (!Preferences::instance()->shell.isNull())
            setShellProgram(Preferences::instance()->shell);
    } else {
        qDebug() << "Settings custom shell program:" << shell;
        QStringList parts = shell.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        qDebug() << parts;
        setShellProgram(parts.at(0));
        parts.removeAt(0);
        if (parts.count())
            setArgs(parts);
    }

    setMotionAfterPasting(Preferences::instance()->m_motionAfterPaste);

    startShellProgram();
}

void TermWidgetImpl::propertiesChanged()
{
    setColorScheme(Preferences::instance()->colorScheme);
    setTerminalFont(Preferences::instance()->font);
    setMotionAfterPasting(Preferences::instance()->m_motionAfterPaste);

    if (Preferences::instance()->historyLimited) {
        setHistorySize(Preferences::instance()->historyLimitedTo);
    } else {
        // Unlimited history
        setHistorySize(-1);
    }

    setKeyBindings(Preferences::instance()->emulation);
    setTerminalOpacity(Preferences::instance()->termOpacity/100.0);

    /* be consequent with qtermwidget.h here */
    switch (Preferences::instance()->scrollBarPos) {
    case 0:
        setScrollBarPosition(QTermWidget::NoScrollBar);
        break;
    case 1:
        setScrollBarPosition(QTermWidget::ScrollBarLeft);
        break;
    case 2:
    default:
        setScrollBarPosition(QTermWidget::ScrollBarRight);
        break;
    }

    update();
}

void TermWidgetImpl::act_splitVertical()
{
    emit splitVertical();
}

void TermWidgetImpl::act_splitHorizontal()
{
    emit splitHorizontal();
}

void TermWidgetImpl::act_splitCollapse()
{
    emit splitCollapse();
}

void TermWidgetImpl::zoomReset()
{
    setTerminalFont(Preferences::instance()->font);
}

TermWidget::TermWidget(const QString &wdir, const QString &shell, QWidget *parent) :
    QWidget(parent)
{
    m_borderColor = palette().color(QPalette::Window);
    m_term = new TermWidgetImpl(wdir, shell, this);
    setFocusProxy(m_term);

    m_layout = new QVBoxLayout();
    setLayout(m_layout);

    m_layout->addWidget(m_term);

    propertiesChanged();

    connect(m_term, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(m_term, SIGNAL(splitHorizontal()), this, SLOT(term_splitHorizontal()));
    connect(m_term, SIGNAL(splitVertical()), this, SLOT(term_splitVertical()));
    connect(m_term, SIGNAL(splitCollapse()), this, SLOT(term_splitCollapse()));
    connect(m_term, SIGNAL(termGetFocus()), this, SLOT(term_termGetFocus()));
    connect(m_term, SIGNAL(termLostFocus()), this, SLOT(term_termLostFocus()));
    connect(m_term, &QTermWidget::urlActivated, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
}

void TermWidget::propertiesChanged()
{
    if (Preferences::instance()->highlightCurrentTerminal)
        m_layout->setContentsMargins(2, 2, 2, 2);
    else
        m_layout->setContentsMargins(0, 0, 0, 0);

    m_term->propertiesChanged();
}

TermWidgetImpl *TermWidget::impl() const
{
    return m_term;
}

void TermWidget::term_splitHorizontal()
{
    emit splitHorizontal(this);
}

void TermWidget::term_splitVertical()
{
    emit splitVertical(this);
}

void TermWidget::term_splitCollapse()
{
    emit splitCollapse(this);
}

void TermWidget::term_termGetFocus()
{
    m_borderColor = palette().color(QPalette::Highlight);
    emit termGetFocus(this);
    update();
}

void TermWidget::term_termLostFocus()
{
    m_borderColor = palette().color(QPalette::Window);
    update();
}

void TermWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QPen pen(m_borderColor);
    pen.setWidth(30);
    pen.setBrush(m_borderColor);
    p.setPen(pen);
    p.drawRect(0, 0, width() - 1, height() - 1);
}
