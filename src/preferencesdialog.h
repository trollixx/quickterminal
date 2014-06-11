#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_preferencesdialog.h"

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

private slots:
    void saveShortcuts();
    void apply();
    void accept() override;

    void changeFontButton_clicked();

private:
    void setFontSample(const QFont &f);
};

#endif // PREFERENCESDIALOG_H
