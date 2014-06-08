#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_preferencesdialog.h"

class PreferencesDialog : public QDialog, public Ui::PreferencesDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

protected:
    void setupShortcuts();
    void saveShortcuts();
    void recordAction(int row, int column);
    void validateAction(int row, int column);

private slots:
    void apply();
    void accept() override;

    void changeFontButton_clicked();
    void bookmarksButton_clicked();

private:
    void setFontSample(const QFont &f);
    void openBookmarksFile(const QString &fname);
    void saveBookmarksFile(const QString &fname);

    QString oldAccelText; // Placeholder when editing shortcut
};

#endif // PREFERENCESDIALOG_H
