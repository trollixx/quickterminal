#include "propertiesdialog.h"

#include "preferences.h"

#include <qtermwidget.h>

#include <QFileDialog>
#include <QFontDialog>
#include <QStyleFactory>

PropertiesDialog::PropertiesDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Preferences::instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Preferences::instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0);

    /* shortcuts */
    setupShortcuts();

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << "No scrollbar" << "Left" << "Right";
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(Preferences::instance()->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << "Top" << "Bottom" << "Left" << "Right";
    tabsPos_comboBox->addItems(tabsPosList);
    tabsPos_comboBox->setCurrentIndex(Preferences::instance()->tabsPos);

    alwaysShowTabsCheckBox->setChecked(Preferences::instance()->alwaysShowTabs);

    // show main menu bar
    showMenuCheckBox->setChecked(Preferences::instance()->menuVisible);

    /* actions by motion after paste */

    QStringList motionAfter;
    motionAfter << "No move" << "Move start" << "Move end";
    motionAfterPasting_comboBox->addItems(motionAfter);
    motionAfterPasting_comboBox->setCurrentIndex(Preferences::instance()->m_motionAfterPaste);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    int ix = styleComboBox->findText(Preferences::instance()->guiStyle);
    if (ix != -1)
        styleComboBox->setCurrentIndex(ix);

    setFontSample(Preferences::instance()->font);

    appOpacityBox->setValue(Preferences::instance()->appOpacity);
    // connect(appOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    termOpacityBox->setValue(Preferences::instance()->termOpacity);
    // connect(termOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    highlightCurrentCheckBox->setChecked(Preferences::instance()->highlightCurrentTerminal);

    askOnExitCheckBox->setChecked(Preferences::instance()->askOnExit);

    useCwdCheckBox->setChecked(Preferences::instance()->useCWD);

    historyLimited->setChecked(Preferences::instance()->historyLimited);
    historyUnlimited->setChecked(!Preferences::instance()->historyLimited);
    historyLimitedTo->setValue(Preferences::instance()->historyLimitedTo);

    dropShowOnStartCheckBox->setChecked(Preferences::instance()->dropShowOnStart);
    dropHeightSpinBox->setValue(Preferences::instance()->dropHeight);
    dropWidthSpinBox->setValue(Preferences::instance()->dropWidht);
    dropShortCutEdit->setText(Preferences::instance()->dropShortCut.toString());

    useBookmarksCheckBox->setChecked(Preferences::instance()->useBookmarks);
    bookmarksLineEdit->setText(Preferences::instance()->bookmarksFile);
    openBookmarksFile(Preferences::instance()->bookmarksFile);
    connect(bookmarksButton, SIGNAL(clicked()),
            this, SLOT(bookmarksButton_clicked()));
}

void PropertiesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PropertiesDialog::apply()
{
    Preferences::instance()->colorScheme = colorSchemaCombo->currentText();
    Preferences::instance()->font = fontSampleLabel->font(); // fontComboBox->currentFont();
    Preferences::instance()->guiStyle = (styleComboBox->currentText() == tr("System Default"))
            ? QString() : styleComboBox->currentText();

    Preferences::instance()->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    Preferences::instance()->appOpacity = qMin(appOpacityBox->value(), 99);

    Preferences::instance()->termOpacity = termOpacityBox->value();
    Preferences::instance()->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    Preferences::instance()->askOnExit = askOnExitCheckBox->isChecked();

    Preferences::instance()->useCWD = useCwdCheckBox->isChecked();

    Preferences::instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();
    Preferences::instance()->tabsPos = tabsPos_comboBox->currentIndex();
    Preferences::instance()->alwaysShowTabs = alwaysShowTabsCheckBox->isChecked();
    Preferences::instance()->menuVisible = showMenuCheckBox->isChecked();
    Preferences::instance()->m_motionAfterPaste = motionAfterPasting_comboBox->currentIndex();

    Preferences::instance()->historyLimited = historyLimited->isChecked();
    Preferences::instance()->historyLimitedTo = historyLimitedTo->value();

    saveShortcuts();

    Preferences::instance()->save();

    Preferences::instance()->dropShowOnStart = dropShowOnStartCheckBox->isChecked();
    Preferences::instance()->dropHeight = dropHeightSpinBox->value();
    Preferences::instance()->dropWidht = dropWidthSpinBox->value();
    Preferences::instance()->dropShortCut = QKeySequence(dropShortCutEdit->text());

    Preferences::instance()->useBookmarks = useBookmarksCheckBox->isChecked();
    Preferences::instance()->bookmarksFile = bookmarksLineEdit->text();
    saveBookmarksFile(Preferences::instance()->bookmarksFile);

    Preferences::instance()->emitChanged();
}

void PropertiesDialog::setFontSample(const QFont &f)
{
    fontSampleLabel->setFont(f);
    QString sample("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void PropertiesDialog::changeFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fontSampleLabel->font(), this,
                                      tr("Select Terminal Font"), QFontDialog::MonospacedFonts);
    if (!ok)
        return;
    setFontSample(font);
}

void PropertiesDialog::saveShortcuts()
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

void PropertiesDialog::setupShortcuts()
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

void PropertiesDialog::recordAction(int row, int column)
{
    oldAccelText = shortcutsWidget->item(row, column)->text();
}

void PropertiesDialog::validateAction(int row, int column)
{
    QTableWidgetItem *item = shortcutsWidget->item(row, column);
    QString accelText = QKeySequence(item->text()).toString();

    if (accelText.isEmpty() && !item->text().isEmpty())
        item->setText(oldAccelText);
    else
        item->setText(accelText);
}

void PropertiesDialog::bookmarksButton_clicked()
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

void PropertiesDialog::openBookmarksFile(const QString &fname)
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

void PropertiesDialog::saveBookmarksFile(const QString &fname)
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
void PropertiesDialog::setupShortcuts()
{
    QList<QString> shortcutKeys = Properties::Instance()->shortcuts.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);

        QLabel *lblShortcut = new QLabel( keyValue, this );
        QPushButton *btnLaunch = new QPushButton( Properties::Instance()->shortcuts.value( keyValue ), this );

        btnLaunch->setObjectName(keyValue);
        connect( btnLaunch, SIGNAL(clicked()), this, SLOT(shortcutPrompt()) );

        shortcutsWidget->setCellWidget( x, 0, lblShortcut );
        shortcutsWidget->setCellWidget( x, 1, btnLaunch );
    }
}

void PropertiesDialog::shortcutPrompt()
{
    QObject *objectSender = sender();

    if( !objectSender )
        return;

    QString name = objectSender->objectName();
    qDebug() << "shortcutPrompt(" << name << ")";

    DialogShortcut *dlgShortcut = new DialogShortcut(this);
    dlgShortcut->setTitle( tr("Select a key sequence for %1").arg(name) );

    QString sequenceString = Properties::Instance()->shortcuts[name];
    dlgShortcut->setKey(sequenceString);

    int result = dlgShortcut->exec();
    if( result == QDialog::Accepted )
    {
        sequenceString = dlgShortcut->getKey();
        Properties::Instance()->shortcuts[name] = sequenceString;
        Properties::Instance()->saveSettings();
    }
}
*/
