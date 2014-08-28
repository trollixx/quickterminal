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

#include "preferencesdialog.h"

#include "actionmanager.h"
#include "preferences.h"

#include <qtermwidget.h>

#include <QFileDialog>
#include <QFontDialog>
#include <QKeyEvent>
#include <QStyleFactory>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    m_preferences(Preferences::instance())
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &PreferencesDialog::apply);
    connect(changeFontButton, &QPushButton::clicked,
            this, &PreferencesDialog::changeFontButton_clicked);

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(m_preferences->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(m_preferences->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0);

    /* scrollbar position */
    scrollBarPos_comboBox->addItems({ tr("No scrollbar"), tr("Left"), tr("Right") });
    scrollBarPos_comboBox->setCurrentIndex(m_preferences->scrollBarPos);

    /* tabs position */
    tabsPos_comboBox->addItems({ tr("Top"), tr("Bottom"), tr("Left"), tr("Right") });
    tabsPos_comboBox->setCurrentIndex(m_preferences->tabsPos);

    alwaysShowTabsCheckBox->setChecked(m_preferences->alwaysShowTabs);

    // show main menu bar
    showMenuCheckBox->setChecked(m_preferences->menuVisible);

    /* actions by motion after paste */
    motionAfterPasting_comboBox->addItems({ tr("No move"), tr("Move start"), tr("Move end") });
    motionAfterPasting_comboBox->setCurrentIndex(m_preferences->m_motionAfterPaste);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(m_preferences->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(m_preferences->font);

    termOpacityBox->setValue(m_preferences->termOpacity);

    highlightCurrentCheckBox->setChecked(m_preferences->highlightCurrentTerminal);

    askOnExitCheckBox->setChecked(m_preferences->askOnExit);

    useCwdCheckBox->setChecked(m_preferences->useCWD);

    historyLimited->setChecked(m_preferences->historyLimited);
    historyUnlimited->setChecked(!m_preferences->historyLimited);
    historyLimitedTo->setValue(m_preferences->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(m_preferences->dropShowOnStart);
    dropHeightSpinBox->setValue(m_preferences->dropHeight);
    dropWidthSpinBox->setValue(m_preferences->dropWidht);

    /// Shortcuts Page
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;

    actionTreeWidget->sortByColumn(0, Qt::AscendingOrder);
    shortcutLineEdit->installEventFilter(this);

    connect(actionTreeWidget, &QTreeWidget::currentItemChanged, this, &PreferencesDialog::selectAction);
    connect(clearShortcutButton, &QPushButton::clicked, this, &PreferencesDialog::clearShortcut);
    connect(resetShortcutButton, &QPushButton::clicked, this, &PreferencesDialog::resetShortcut);

    QFont userEditedShortcutFont;
    userEditedShortcutFont.setBold(true);
    userEditedShortcutFont.setItalic(true);

    // Populate shortcuts
    foreach (const ActionInfo &actionInfo, ActionManager::registry()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(actionTreeWidget);
        item->setText(0, ActionManager::clearActionText(actionInfo.text));
        item->setData(0, Qt::UserRole, actionInfo.id);
        if (actionInfo.shortcut != actionInfo.defaultShortcut)
            item->setFont(1, userEditedShortcutFont);
        item->setText(1, actionInfo.shortcut.toString(QKeySequence::NativeText));
    }
}

void PreferencesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PreferencesDialog::apply()
{
    m_preferences->colorScheme = colorSchemaCombo->currentText();
    m_preferences->font = fontSampleLabel->font(); // fontComboBox->currentFont();
    m_preferences->guiStyle = (styleComboBox->currentText() == tr("System Default"))
            ? QString() : styleComboBox->currentText();

    m_preferences->emulation = emulationComboBox->currentText();

    m_preferences->termOpacity = termOpacityBox->value();
    m_preferences->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    m_preferences->askOnExit = askOnExitCheckBox->isChecked();

    m_preferences->useCWD = useCwdCheckBox->isChecked();

    m_preferences->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    m_preferences->tabsPos = tabsPos_comboBox->currentIndex();
    m_preferences->alwaysShowTabs = alwaysShowTabsCheckBox->isChecked();
    m_preferences->menuVisible = showMenuCheckBox->isChecked();
    m_preferences->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();

    m_preferences->historyLimited = historyLimited->isChecked();
    m_preferences->historyLimitedTo = historyLimitedTo->value();

    applyShortcuts();

    m_preferences->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    m_preferences->dropHeight = dropHeightSpinBox->value();
    m_preferences->dropWidht = dropWidthSpinBox->value();

    m_preferences->save();

    m_preferences->emitChanged();
}

bool PreferencesDialog::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        handleKeyEvent(k);
        return true;
    }

    if (event->type() == QEvent::Shortcut || event->type() == QEvent::KeyRelease)
        return true;

    if (event->type() == QEvent::ShortcutOverride) {
        event->accept();
        return true;
    }

    return false;
}

void PreferencesDialog::selectAction(QTreeWidgetItem *item)
{
    shortcutEditGroupBox->setEnabled(true);

    const QString actionId = item->data(0, Qt::UserRole).toString();
    const QKeySequence ks = ActionManager::actionInfo(actionId).shortcut;

    m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    m_keyNum = ks.count();
    for (int i = 0; i < m_keyNum; ++i)
        m_key[i] = ks[i];
    updateCurrentShortcut(ks);
}

void PreferencesDialog::clearShortcut()
{
    m_keyNum = m_key[0] = m_key[1] = m_key[2] = m_key[3] = 0;
    updateCurrentShortcut(QKeySequence());
}

void PreferencesDialog::resetShortcut()
{
    QTreeWidgetItem *item = actionTreeWidget->currentItem();
    const QString actionId = item->data(0, Qt::UserRole).toString();
    const QKeySequence ks = ActionManager::actionInfo(actionId).defaultShortcut;
    updateCurrentShortcut(ks);
    shortcutLineEdit->setText(ks.toString(QKeySequence::NativeText));
}

void PreferencesDialog::applyShortcuts()
{
    for (int i = 0; i < actionTreeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = actionTreeWidget->topLevelItem(i);
        const QString actionId = item->data(0, Qt::UserRole).toString();
        const QKeySequence ks(item->text(1));
        ActionManager::updateShortcut(actionId, ks);
        m_preferences->setShortcut(actionId, ks);
    }
}

void PreferencesDialog::handleKeyEvent(QKeyEvent *event)
{
    int nextKey = event->key();
    if (m_keyNum > 3 || nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift
            || nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt)
        return;

    nextKey |= translateModifiers(event->modifiers(), event->text());
    switch (m_keyNum) {
    case 0:
        m_key[0] = nextKey;
        break;
    case 1:
        m_key[1] = nextKey;
        break;
    case 2:
        m_key[2] = nextKey;
        break;
    case 3:
        m_key[3] = nextKey;
        break;
    default:
        break;
    }
    ++m_keyNum;
    QKeySequence ks(m_key[0], m_key[1], m_key[2], m_key[3]);
    updateCurrentShortcut(ks);
    event->accept();
}

int PreferencesDialog::translateModifiers(Qt::KeyboardModifiers state, const QString &text)
{
    int result = 0;
    // The shift modifier only counts when it is not used to type a symbol
    // that is only reachable using the shift key anyway
    if ((state & Qt::ShiftModifier)
            && (text.size() == 0 || !text.at(0).isPrint()
                || text.at(0).isLetterOrNumber() || text.at(0).isSpace())) {
        result |= Qt::SHIFT;
    }
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

void PreferencesDialog::updateCurrentShortcut(const QKeySequence &ks)
{
    shortcutLineEdit->setText(ks.toString(QKeySequence::NativeText));
    QTreeWidgetItem *item = actionTreeWidget->currentItem();
    item->setText(1, ks.toString(QKeySequence::NativeText));
}

void PreferencesDialog::changeFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fontSampleLabel->font(), this,
                                      tr("Select Terminal Font"), QFontDialog::MonospacedFonts);
    if (!ok)
        return;
    setFontSample(font);
}

void PreferencesDialog::setFontSample(const QFont &f)
{
    fontSampleLabel->setFont(f);
    QString sample(QStringLiteral("%1 %2 pt"));
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}
