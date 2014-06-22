#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_preferencesdialog.h"

class QKeyEvent;
class QTreeWidgetItem;

class Preferences;

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

protected:
    void accept() override;
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void selectAction(QTreeWidgetItem *item);
    void clearShortcut();
    void resetShortcut();

private:
    void apply();
    void applyShortcuts();

    void handleKeyEvent(QKeyEvent *event);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void updateCurrentShortcut(const QKeySequence &ks);

    void changeFontButton_clicked();
    void setFontSample(const QFont &f);

    Preferences * const m_preferences = nullptr;
    int m_key[4];
    int m_keyNum;
};

#endif // PREFERENCESDIALOG_H
