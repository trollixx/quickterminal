#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

#include <QMap>

class QAction;
class QMenu;

class TermWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TermWidget(const QString &wdir, const QString &shell = QString(),
                        QWidget *parent = nullptr);

    void propertiesChanged();

    QTermWidget *impl() const;
    void zoomReset();

signals:
    void finished();
    void renameSession();
    void removeCurrentSession();
    void termGetFocus(TermWidget *self);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void term_termGetFocus();
    void term_termLostFocus();

private:
    QTermWidget *m_term = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QColor m_borderColor;
};

#endif // TERMWIDGET_H
