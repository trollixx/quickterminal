#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_preferencesdialog.h"

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

protected:
    void saveShortcuts();
    void recordAction(int row, int column);
    void validateAction(int row, int column);

private slots:
    void apply();
    void accept() override;

    void changeFontButton_clicked();

private:
    void setFontSample(const QFont &f);

    QString oldAccelText; // Placeholder when editing shortcut
};

#endif // PREFERENCESDIALOG_H
