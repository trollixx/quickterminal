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
    void apply();
    void changeFontButton_clicked();

    void selectAction(QTreeWidgetItem *item);
    void resetShortcut();

private:
    void applyShortcuts();

    void handleKeyEvent(QKeyEvent *event);
    int translateModifiers(Qt::KeyboardModifiers state, const QString &text);
    void updateCurrentShortcut(const QKeySequence &ks);

    void setFontSample(const QFont &f);

    Preferences * const m_preferences = nullptr;
    int m_key[4];
    int m_keyNum;
};

#endif // PREFERENCESDIALOG_H
