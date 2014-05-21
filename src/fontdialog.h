#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include "ui_fontdialog.h"

#include "properties.h"

class FontDialog : public QDialog, public Ui::FontDialog
{
    Q_OBJECT
public:
    FontDialog(const QFont &f);
    QFont getFont();

private slots:
    void setFontSample();
};

#endif // FONTDIALOG_H
