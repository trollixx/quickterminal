#include "termwidget.h"

#include "preferences.h"

#include <QDesktopServices>
#include <QPainter>
#include <QVBoxLayout>

namespace {
const bool FlowControlEnabled = false;
const bool FlowControlWarningEnabled = false;
}

TermWidget::TermWidget(const QString &workingDir, const QString &command, QWidget *parent) :
    QWidget(parent),
    m_preferences(Preferences::instance())
{
    m_borderColor = palette().color(QPalette::Window);

    m_layout = new QVBoxLayout();
    setLayout(m_layout);

    m_term = new QTermWidget(0, this);
    m_term->setFlowControlEnabled(FlowControlEnabled);
    m_term->setFlowControlWarningEnabled(FlowControlWarningEnabled);

    if (!workingDir.isNull())
        m_term->setWorkingDirectory(workingDir);

    if (command.isNull()) {
        if (!m_preferences->shell.isNull())
            m_term->setShellProgram(m_preferences->shell);
    } else {
        QStringList parts = command.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        m_term->setShellProgram(parts.first());
        parts.removeFirst();
        if (parts.count())
            m_term->setArgs(parts);
    }

    m_term->setMotionAfterPasting(m_preferences->m_motionAfterPaste);
    m_term->startShellProgram();

    setFocusProxy(m_term);
    m_layout->addWidget(m_term);

    propertiesChanged();

    connect(m_term, &QTermWidget::finished, this, &TermWidget::finished);
    connect(m_term, &QTermWidget::termGetFocus, this, &TermWidget::term_termGetFocus);
    connect(m_term, &QTermWidget::termLostFocus, this, &TermWidget::term_termLostFocus);
    connect(m_term, &QTermWidget::urlActivated, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
}

void TermWidget::propertiesChanged()
{
    if (m_preferences->highlightCurrentTerminal)
        m_layout->setContentsMargins(2, 2, 2, 2);
    else
        m_layout->setContentsMargins(0, 0, 0, 0);

    m_term->setColorScheme(m_preferences->colorScheme);
    m_term->setTerminalFont(m_preferences->font);
    m_term->setMotionAfterPasting(m_preferences->m_motionAfterPaste);
    m_term->setHistorySize(m_preferences->historyLimited ? m_preferences->historyLimitedTo : -1);
    m_term->setKeyBindings(m_preferences->emulation);
    m_term->setTerminalOpacity(m_preferences->termOpacity / 100.0);
    m_term->setScrollBarPosition(
                static_cast<QTermWidget::ScrollBarPosition>(m_preferences->scrollBarPos));
    m_term->update();
}

QTermWidget *TermWidget::impl() const
{
    return m_term;
}

void TermWidget::zoomReset()
{
    m_term->setTerminalFont(m_preferences->font);
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
