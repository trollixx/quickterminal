#include "preferencesdialog.h"

#include "actionmanager.h"
#include "preferences.h"

#include <qtermwidget.h>

#include <QFileDialog>
#include <QFontDialog>
#include <QKeyEvent>
#include <QStyleFactory>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    const Preferences * const preferences = Preferences::instance();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(preferences->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(preferences->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0);

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << "No scrollbar" << "Left" << "Right";
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(preferences->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << "Top" << "Bottom" << "Left" << "Right";
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(preferences->tabsPos);

    alwaysShowTabsCheckBox->setChecked(preferences->alwaysShowTabs);

    // show main menu bar
    showMenuCheckBox->setChecked(preferences->menuVisible);

    /* actions by motion after paste */

    QStringList motionAfter;
    motionAfter << "No move" << "Move start" << "Move end";
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(preferences->m_motionAfterPaste);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(preferences->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(preferences->font);

    appOpacityBox->setValue(preferences->appOpacity);
    // connect(appOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    termOpacityBox->setValue(preferences->termOpacity);
    // connect(termOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    highlightCurrentCheckBox->setChecked(preferences->highlightCurrentTerminal);

    askOnExitCheckBox->setChecked(preferences->askOnExit);

    useCwdCheckBox->setChecked(preferences->useCWD);

    historyLimited->setChecked(preferences->historyLimited);
    historyUnlimited->setChecked(!preferences->historyLimited);
    historyLimitedTo->setValue(preferences->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(preferences->dropShowOnStart);
    dropHeightSpinBox->setValue(preferences->dropHeight);
    dropWidthSpinBox->setValue(preferences->dropWidht);
    dropShortCutEdit->setText(preferences->dropShortCut.toString());

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
    Preferences * const preferences = Preferences::instance();
    preferences->colorScheme = colorSchemaCombo->currentText();
    preferences->font = fontSampleLabel->font(); // fontComboBox->currentFont();
    preferences->guiStyle = (styleComboBox->currentText() == tr("System Default"))
            ? QString() : styleComboBox->currentText();

    preferences->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    preferences->appOpacity = qMin(appOpacityBox->value(), 99);

    preferences->termOpacity = termOpacityBox->value();
    preferences->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    preferences->askOnExit = askOnExitCheckBox->isChecked();

    preferences->useCWD = useCwdCheckBox->isChecked();

    preferences->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    preferences->tabsPos = tabsPos_comboBox->currentIndex();
    preferences->alwaysShowTabs = alwaysShowTabsCheckBox->isChecked();
    preferences->menuVisible = showMenuCheckBox->isChecked();
    preferences->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();

    preferences->historyLimited = historyLimited->isChecked();
    preferences->historyLimitedTo = historyLimitedTo->value();

    applyShortcuts();

    preferences->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    preferences->dropHeight = dropHeightSpinBox->value();
    preferences->dropWidht = dropWidthSpinBox->value();
    preferences->dropShortCut = QKeySequence(dropShortCutEdit->text());

    preferences->save();

    preferences->emitChanged();
}

bool PreferencesDialog::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *k = static_cast<QKeyEvent*>(event);
        handleKeyEvent(k);
        return true;
    }

    if (event->type() == QEvent::Shortcut || event->type() == QEvent::KeyRelease )
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
        ActionManager::updateShortcut(actionId, QKeySequence(item->text(1)));
    }
}

void PreferencesDialog::handleKeyEvent(QKeyEvent *event)
{
    int nextKey = event->key();
    if (m_keyNum > 3 ||
            nextKey == Qt::Key_Control ||
            nextKey == Qt::Key_Shift ||
            nextKey == Qt::Key_Meta ||
            nextKey == Qt::Key_Alt)
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
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetterOrNumber()
                                        || text.at(0).isSpace()))
        result |= Qt::SHIFT;
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
    QString sample("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}
