#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QHash>
#include <QIcon>
#include <QKeySequence>
#include <QObject>

class QAction;
class QWidget;

class ActionManager : public QObject
{
    Q_OBJECT
public:
    explicit ActionManager(QWidget *parent = nullptr);
    ~ActionManager() override;

    void addAction(const QString &id, QAction *action);
    QAction *addAction(const QString &id, const QString &text,
                       const QKeySequence &shortcut = QKeySequence(),
                       const QIcon &icon = QIcon());
    QAction *action(const QString &id) const;

    static ActionManager *instance(QWidget *widget);
    static void updateShortcut(const QString &id, const QKeySequence &shortcut);

signals:
    void changed(const QString &id);

private:
    static QList<ActionManager *> m_instances;

    QHash<QString, QAction *> m_actions;
};

#endif // ACTIONMANAGER_H
