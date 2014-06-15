#include "actionmanager.h"

#include "preferences.h"

#include <QAction>

QHash<QString, ActionInfo> ActionManager::m_actionRegistry;
QList<ActionManager *> ActionManager::m_instances;

ActionManager::ActionManager(QWidget *parent) :
    QObject(parent)
{
    m_instances.append(this);
}

ActionManager::~ActionManager()
{
    qDeleteAll(m_actions);
    m_instances.removeAll(this);
}

QAction *ActionManager::action(const QString &id) const
{
    if (!m_actionRegistry.contains(id))
        return nullptr;

    if (!m_actions.contains(id)) {
        /// TODO: Find a less hacky solution, should it really be a const method?
        ActionManager *self = const_cast<ActionManager *>(this);
        const ActionInfo actionInfo = m_actionRegistry.value(id);
        QAction *action = new QAction(actionInfo.icon, actionInfo.text, self);
        action->setShortcut(actionInfo.shortcut);
        self->m_actions.insert(id, action);
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

QList<ActionInfo> ActionManager::registry()
{
    return m_actionRegistry.values();
}

ActionInfo ActionManager::actionInfo(const QString &id)
{
    return m_actionRegistry.value(id);
}

bool ActionManager::registerAction(const QString &id, const ActionInfo &info)
{
    if (m_actionRegistry.contains(id)) {
        qWarning("Action '%s' has been already registered.", qPrintable(id));
        return false;
    }
    m_actionRegistry.insert(id, info);
    return true;
}

bool ActionManager::registerAction(const QString &id, const QString &text,
                                   const QKeySequence &defaultShortcut, const QIcon &icon)
{
    ActionInfo actionInfo;
    actionInfo.id = id;
    actionInfo.text = text;
    actionInfo.defaultShortcut = actionInfo.shortcut = defaultShortcut;
    actionInfo.icon = icon;
    return registerAction(id, actionInfo);
}

bool ActionManager::registerAction(const QString &id, const QString &text, const QIcon &icon)
{
    return registerAction(id, text, QKeySequence(), icon);
}

void ActionManager::updateShortcut(const QString &id, const QKeySequence &shortcut)
{
    if (!m_actionRegistry.contains(id))
        return;

    ActionInfo actionInfo = m_actionRegistry.value(id);

    if (shortcut == actionInfo.shortcut)
        return;

    actionInfo.shortcut = shortcut;
    m_actionRegistry.insert(id, actionInfo);

    foreach (ActionManager *am, m_instances) {
        if (!am->m_actions.contains(id)) {
            // This should never happen
            qCritical("Action '%s' not found!", qPrintable(id));
            continue;
        }
        am->m_actions.value(id)->setShortcut(shortcut);
    }
}

QString ActionManager::clearActionText(const QString &text)
{
    QString str(text);
    /// TODO: Handle multiple ampersands
    return str.remove(QStringLiteral("&"));
}
