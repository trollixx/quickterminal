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

#include "terminalwidget.h"

#include "preferences.h"

#include <QDesktopServices>
#include <QPainter>
#include <QVBoxLayout>

namespace {
const bool FlowControlEnabled = false;
const bool FlowControlWarningEnabled = false;
}

TerminalWidget::TerminalWidget(const QString &workingDir, const QString &command, QWidget *parent) :
    QTermWidget(0, parent),
    m_preferences(Preferences::instance())
{
    setFlowControlEnabled(FlowControlEnabled);
    setFlowControlWarningEnabled(FlowControlWarningEnabled);

    if (!workingDir.isNull())
        setWorkingDirectory(workingDir);

    if (command.isNull()) {
        if (!m_preferences->shell.isNull())
            setShellProgram(m_preferences->shell);
    } else {
        QStringList parts = command.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        setShellProgram(parts.first());
        parts.removeFirst();
        if (parts.count())
            setArgs(parts);
    }

    setMotionAfterPasting(m_preferences->m_motionAfterPaste);
    startShellProgram();

    propertiesChanged();

    connect(this, &QTermWidget::finished, this, &TerminalWidget::finished);
    connect(this, &QTermWidget::termGetFocus, this, [this]() {
        emit focused(this);
    });
    connect(this, &QTermWidget::urlActivated, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
}

void TerminalWidget::propertiesChanged()
{
    setColorScheme(m_preferences->colorScheme);
    setTerminalFont(m_preferences->font);
    setMotionAfterPasting(m_preferences->m_motionAfterPaste);
    setHistorySize(m_preferences->historyLimited ? m_preferences->historyLimitedTo : -1);
    setKeyBindings(m_preferences->emulation);
    setTerminalOpacity(m_preferences->termOpacity / 100.0);
    setScrollBarPosition(
                static_cast<QTermWidget::ScrollBarPosition>(m_preferences->scrollBarPos));
    update();
}

void TerminalWidget::zoomReset()
{
    setTerminalFont(m_preferences->font);
}
