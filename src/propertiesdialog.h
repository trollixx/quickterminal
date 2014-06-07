#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "ui_propertiesdialog.h"

class PropertiesDialog : public QDialog, Ui::PropertiesDialog
{
    Q_OBJECT
public:
    explicit PropertiesDialog(QWidget *parent = nullptr);

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

#endif // PROPERTIESDIALOG_H
