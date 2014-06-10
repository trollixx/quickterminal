#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

#include <QMap>

class QAction;
class QMenu;

class TermWidgetImpl : public QTermWidget
{
    Q_OBJECT
public:
    explicit TermWidgetImpl(const QString &wdir, const QString &shell = QString(),
                   QWidget *parent = nullptr);
    void propertiesChanged();

signals:
    void renameSession();
    void removeCurrentSession();
    void splitHorizontal();
    void splitVertical();
    void splitCollapse();

public slots:
    void zoomReset();

private slots:
    void act_splitVertical();
    void act_splitHorizontal();
    void act_splitCollapse();
};

class TermWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TermWidget(const QString &wdir, const QString &shell = QString(),
                        QWidget *parent = nullptr);

    void propertiesChanged();

    TermWidgetImpl *impl() const;

signals:
    void finished();
    void renameSession();
    void removeCurrentSession();
    void splitHorizontal(TermWidget *self);
    void splitVertical(TermWidget *self);
    void splitCollapse(TermWidget *self);
    void termGetFocus(TermWidget *self);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void term_splitHorizontal();
    void term_splitVertical();
    void term_splitCollapse();
    void term_termGetFocus();
    void term_termLostFocus();

private:
    TermWidgetImpl *m_term = nullptr;
    QVBoxLayout *m_layout = nullptr;
    QColor m_borderColor;
};

#endif // TERMWIDGET_H
