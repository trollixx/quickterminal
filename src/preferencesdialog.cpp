#include "preferencesdialog.h"

#include "preferences.h"

#include <qtermwidget.h>

#include <QFileDialog>
#include <QFontDialog>
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

    /* shortcuts */
    setupShortcuts();

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

    saveShortcuts();

    preferences->save();

    preferences->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    preferences->dropHeight = dropHeightSpinBox->value();
    preferences->dropWidht = dropWidthSpinBox->value();
    preferences->dropShortCut = QKeySequence(dropShortCutEdit->text());

    preferences->emitChanged();
}

void PreferencesDialog::setFontSample(const QFont &f)
{
    fontSampleLabel->setFont(f);
    QString sample("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
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

void PreferencesDialog::saveShortcuts()
{
    QList<QString> shortcutKeys = Preferences::instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount(shortcutCount);

    for (int x = 0; x < shortcutCount; x++) {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Preferences::instance()->actions[keyValue];

        QTableWidgetItem *item = shortcutsWidget->item(x, 1);
        QKeySequence sequence = QKeySequence(item->text());
        QString sequenceString = sequence.toString();

        keyAction->setShortcut(sequenceString);
    }
}

void PreferencesDialog::setupShortcuts()
{
    QList<QString> shortcutKeys = Preferences::instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount(shortcutCount);

    for (int x = 0; x < shortcutCount; x++) {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Preferences::instance()->actions[keyValue];

        QTableWidgetItem *itemName = new QTableWidgetItem(tr(keyValue.toStdString().c_str()));
        QTableWidgetItem *itemShortcut = new QTableWidgetItem(keyAction->shortcut().toString());

        itemName->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        shortcutsWidget->setItem(x, 0, itemName);
        shortcutsWidget->setItem(x, 1, itemShortcut);
    }

    shortcutsWidget->resizeColumnsToContents();
    /*
    connect(shortcutsWidget, SIGNAL(currentChanged(int, int)),
            this, SLOT(recordAction(int, int)));
    connect(shortcutsWidget, SIGNAL(valueChanged(int, int)),
            this, SLOT(validateAction(int, int)));
*/
}

void PreferencesDialog::recordAction(int row, int column)
{
    oldAccelText = shortcutsWidget->item(row, column)->text();
}

void PreferencesDialog::validateAction(int row, int column)
{
    QTableWidgetItem *item = shortcutsWidget->item(row, column);
    QString accelText = QKeySequence(item->text()).toString();

    if (accelText.isEmpty() && !item->text().isEmpty())
        item->setText(oldAccelText);
    else
        item->setText(accelText);
}

void PreferencesDialog::bookmarksButton_clicked()
{
    QFileDialog dia(this, tr("Open or create bookmarks file"));
    dia.setConfirmOverwrite(false);
    dia.setFileMode(QFileDialog::AnyFile);
    if (!dia.exec())
        return;

    QString fname = dia.selectedFiles().count() ? dia.selectedFiles().at(0) : QString();
    if (fname.isNull())
        return;

    bookmarksLineEdit->setText(fname);
    openBookmarksFile(bookmarksLineEdit->text());
}

void PreferencesDialog::openBookmarksFile(const QString &fname)
{
    QFile f(fname);
    QString content;
    if (!f.open(QFile::ReadOnly))
        content = "<qterminal>\n  <group name\"group1\">\n    <command name=\"cmd1\" value=\"cd $HOME\"/>\n  </group>\n</qterminal>";
    else
        content = f.readAll();

    bookmarkPlainEdit->setPlainText(content);
    bookmarkPlainEdit->document()->setModified(false);
}

void PreferencesDialog::saveBookmarksFile(const QString &fname)
{
    if (!bookmarkPlainEdit->document()->isModified())
        return;

    QFile f(fname);
    if (!f.open(QFile::WriteOnly | QFile::Truncate)) {
        qDebug("Cannot open file '%s'", qPrintable(f.fileName()));
        return;
    }

    f.write(bookmarkPlainEdit->toPlainText().toUtf8());
}

/*
void PreferencesDialog::setupShortcuts()
{
    QList<QString> shortcutKeys = Preferences::instance()->shortcuts.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);

        QLabel *lblShortcut = new QLabel( keyValue, this );
        QPushButton *btnLaunch = new QPushButton( Preferences::instance()->shortcuts.value( keyValue ), this );

        btnLaunch->setObjectName(keyValue);
        connect( btnLaunch, SIGNAL(clicked()), this, SLOT(shortcutPrompt()) );

        shortcutsWidget->setCellWidget( x, 0, lblShortcut );
        shortcutsWidget->setCellWidget( x, 1, btnLaunch );
    }
}

void PreferencesDialog::shortcutPrompt()
{
    QObject *objectSender = sender();

    if( !objectSender )
        return;

    QString name = objectSender->objectName();
    qDebug() << "shortcutPrompt(" << name << ")";

    DialogShortcut *dlgShortcut = new DialogShortcut(this);
    dlgShortcut->setTitle( tr("Select a key sequence for %1").arg(name) );

    QString sequenceString = Preferences::instance()->shortcuts[name];
    dlgShortcut->setKey(sequenceString);

    int result = dlgShortcut->exec();
    if( result == QDialog::Accepted )
    {
        sequenceString = dlgShortcut->getKey();
        Preferences::instance()->shortcuts[name] = sequenceString;
        Preferences::instance()->saveSettings();
    }
}
*/
