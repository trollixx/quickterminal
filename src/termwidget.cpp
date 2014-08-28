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

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

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
    layout->addWidget(m_term);

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
        layout()->setContentsMargins(2, 2, 2, 2);
    else
        layout()->setContentsMargins(0, 0, 0, 0);

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
