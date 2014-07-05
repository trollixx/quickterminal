#include "termwidget.h"

#include "preferences.h"

#include <QDebug>
#include <QDesktopServices>
#include <QPainter>
#include <QVBoxLayout>

#define FLOW_CONTROL_ENABLED false
#define FLOW_CONTROL_WARNING_ENABLED false

static int TermWidgetCount = 0;

TermWidget::TermWidget(const QString &wdir, const QString &shell, QWidget *parent) :
    QWidget(parent)
{
    m_borderColor = palette().color(QPalette::Window);

    m_layout = new QVBoxLayout();
    setLayout(m_layout);

    m_term = new QTermWidget(0, this);
    TermWidgetCount++;
    QString name("TermWidget_%1");
    m_term->setObjectName(name.arg(TermWidgetCount));

    m_term->setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    m_term->setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);

    if (!wdir.isNull())
        m_term->setWorkingDirectory(wdir);

    if (shell.isNull()) {
        if (!Preferences::instance()->shell.isNull())
            m_term->setShellProgram(Preferences::instance()->shell);
    } else {
        qDebug() << "Settings custom shell program:" << shell;
        QStringList parts = shell.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        qDebug() << parts;
        m_term->setShellProgram(parts.at(0));
        parts.removeAt(0);
        if (parts.count())
            m_term->setArgs(parts);
    }

    m_term->setMotionAfterPasting(Preferences::instance()->m_motionAfterPaste);
    m_term->startShellProgram();

    setFocusProxy(m_term);
    m_layout->addWidget(m_term);

    propertiesChanged();

    connect(m_term, SIGNAL(finished()), this, SIGNAL(finished()));
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

    m_term->setColorScheme(Preferences::instance()->colorScheme);
    m_term->setTerminalFont(Preferences::instance()->font);
    m_term->setMotionAfterPasting(Preferences::instance()->m_motionAfterPaste);

    if (Preferences::instance()->historyLimited) {
        m_term->setHistorySize(Preferences::instance()->historyLimitedTo);
    } else {
        // Unlimited history
        m_term->setHistorySize(-1);
    }

    m_term->setKeyBindings(Preferences::instance()->emulation);
    m_term->setTerminalOpacity(Preferences::instance()->termOpacity/100.0);

    /* be consequent with qtermwidget.h here */
    switch (Preferences::instance()->scrollBarPos) {
    case 0:
        m_term->setScrollBarPosition(QTermWidget::NoScrollBar);
        break;
    case 1:
        m_term->setScrollBarPosition(QTermWidget::ScrollBarLeft);
        break;
    case 2:
    default:
        m_term->setScrollBarPosition(QTermWidget::ScrollBarRight);
        break;
    }

    m_term->update();
}

QTermWidget *TermWidget::impl() const
{
    return m_term;
}

void TermWidget::zoomReset()
{
    m_term->setTerminalFont(Preferences::instance()->font);
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
