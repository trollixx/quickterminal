#include "actionmanager.h"

#include "preferences.h"

#include <QAction>

QList<ActionManager *> ActionManager::m_instances;

ActionManager::ActionManager(QWidget *parent) :
    QObject(parent)
{
    m_instances.append(this);
}

ActionManager::~ActionManager()
{
    m_instances.removeAll(this);
}

void ActionManager::addAction(const QString &id, QAction *action)
{
    Q_ASSERT(!m_actions.contains(id));
    action->setParent(this);
    m_actions.insert(id, action);
}

QAction *ActionManager::addAction(const QString &id, const QString &text,
                                  const QKeySequence &shortcut, const QIcon &icon)
{
    QAction *action = new QAction(icon, text, this);
    action->setShortcut(shortcut);
    addAction(id, action);
    return action;
}

QAction *ActionManager::action(const QString &id) const
{
    if (!m_actions.contains(id)) {
        qWarning("Action '%s' not found!", qPrintable(id));
        return nullptr;
    }

    return m_actions.value(id);
}

ActionManager *ActionManager::instance(QWidget *widget)
{
    foreach (ActionManager *am, m_instances) {
        QWidget *parent = widget;
        while (parent != nullptr) {
            if (parent == am->parent())
                return am;
            parent = parent->parentWidget();
        }
    }
    return nullptr;
}

void ActionManager::updateShortcut(const QString &id, const QKeySequence &shortcut)
{
    foreach (ActionManager *am, m_instances) {
        if (!am->m_actions.contains(id)) {
            // This should never happen
            qCritical("Action '%s' not found!", qPrintable(id));
            continue;
        }
        am->m_actions.value(id)->setShortcut(shortcut);
    }
}
