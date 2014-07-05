#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

class Preferences;

class TermWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TermWidget(const QString &workingDir, const QString &command = QString(),
                        QWidget *parent = nullptr);

    void propertiesChanged();

    QTermWidget *impl() const;
    void zoomReset();

signals:
    void finished();
    void termGetFocus(TermWidget *self);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void term_termGetFocus();
    void term_termLostFocus();

private:
    Preferences * const m_preferences = nullptr;

    QTermWidget *m_term = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QColor m_borderColor;
};

#endif // TERMWIDGET_H
