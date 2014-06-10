#include "preferences.h"

#include <QApplication>
#include <QSettings>

namespace {
const char DefaultFont[] = "Monospace";
}

Preferences *Preferences::m_instance = nullptr;

Preferences *Preferences::instance()
{
    if (!m_instance)
        m_instance = new Preferences();
    return m_instance;
}

Preferences::Preferences(QObject *parent) :
    QObject(parent),
    m_settings(new QSettings(this))
{
    load();
}

Preferences::~Preferences()
{
    qDebug("Preferences destructor called");
    save();
    m_instance = nullptr;
}

void Preferences::load()
{
    guiStyle = m_settings->value("guiStyle", QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme = m_settings->value("colorScheme", "Linux").toString();

    highlightCurrentTerminal = m_settings->value("highlightCurrentTerminal", true).toBool();

    font = qvariant_cast<QFont>(m_settings->value("font", defaultFont()));

    m_settings->beginGroup("Shortcuts");
    foreach (const QString &key, m_settings->childKeys())
        m_shortcuts.insert(key, m_settings->value(key).toString());
    m_settings->endGroup();

    mainWindowGeometry = m_settings->value("MainWindow/geometry").toByteArray();
    mainWindowState = m_settings->value("MainWindow/state").toByteArray();

    historyLimited = m_settings->value("HistoryLimited", true).toBool();
    historyLimitedTo = m_settings->value("HistoryLimitedTo", 1000).toUInt();

    emulation = m_settings->value("emulation", "default").toString();

    // sessions
    int size = m_settings->beginReadArray("Sessions");
    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);
        QString name(m_settings->value("name").toString());
        if (name.isEmpty())
            continue;
        sessions[name] = m_settings->value("state").toByteArray();
    }
    m_settings->endArray();

    appOpacity = m_settings->value("MainWindow/appOpacity", 100).toInt();
    termOpacity = m_settings->value("termOpacity", 100).toInt();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = m_settings->value("ScrollbarPosition", 2).toInt();
    /* default to North. I'd prefer South but North is standard (they say) */
    tabsPos = m_settings->value("TabsPosition", 0).toInt();
    alwaysShowTabs = m_settings->value("AlwaysShowTabs", true).toBool();
    m_motionAfterPaste = m_settings->value("MotionAfterPaste", 0).toInt();

    /* toggles */
    tabBarless = m_settings->value("TabBarless", false).toBool();
    menuVisible = m_settings->value("MenuVisible", true).toBool();
    askOnExit = m_settings->value("AskOnExit", true).toBool();
    useCWD = m_settings->value("UseCWD", false).toBool();

    m_settings->beginGroup("DropMode");
    dropShortCut = QKeySequence(m_settings->value("ShortCut", "F12").toString());
    dropKeepOpen = m_settings->value("KeepOpen", false).toBool();
    dropShowOnStart = m_settings->value("ShowOnStart", true).toBool();
    dropWidht = m_settings->value("Width", 70).toInt();
    dropHeight = m_settings->value("Height", 45).toInt();
    m_settings->endGroup();
}

void Preferences::save()
{
    m_settings->setValue("guiStyle", guiStyle);
    m_settings->setValue("colorScheme", colorScheme);
    m_settings->setValue("highlightCurrentTerminal", highlightCurrentTerminal);
    m_settings->setValue("font", font);

    m_settings->beginGroup("Shortcuts");
    for (auto it = m_shortcuts.cbegin(); it != m_shortcuts.cend(); ++it)
        m_settings->setValue(it.key(), it.value());
    m_settings->endGroup();

    m_settings->setValue("MainWindow/geometry", mainWindowGeometry);
    m_settings->setValue("MainWindow/state", mainWindowState);

    m_settings->setValue("HistoryLimited", historyLimited);
    m_settings->setValue("HistoryLimitedTo", historyLimitedTo);

    m_settings->setValue("emulation", emulation);

    // sessions
    m_settings->beginWriteArray("Sessions");
    int i = 0;
    auto sit = sessions.begin();
    while (sit != sessions.end()) {
        m_settings->setArrayIndex(i);
        m_settings->setValue("name", sit.key());
        m_settings->setValue("state", sit.value());
        ++sit;
        ++i;
    }
    m_settings->endArray();

    m_settings->setValue("MainWindow/appOpacity", appOpacity);
    m_settings->setValue("termOpacity", termOpacity);
    m_settings->setValue("ScrollbarPosition", scrollBarPos);
    m_settings->setValue("TabsPosition", tabsPos);
    m_settings->setValue("AlwaysShowTabs", alwaysShowTabs);
    m_settings->setValue("MotionAfterPaste", m_motionAfterPaste);
    m_settings->setValue("TabBarless", tabBarless);
    m_settings->setValue("MenuVisible", menuVisible);
    m_settings->setValue("AskOnExit", askOnExit);
    m_settings->setValue("UseCWD", useCWD);

    m_settings->beginGroup("DropMode");
    m_settings->setValue("ShortCut", dropShortCut.toString());
    m_settings->setValue("KeepOpen", dropKeepOpen);
    m_settings->setValue("ShowOnStart", dropShowOnStart);
    m_settings->setValue("Width", dropWidht);
    m_settings->setValue("Height", dropHeight);
    m_settings->endGroup();
}

QFont Preferences::defaultFont() const
{
    QFont font = QApplication::font();
    font.setFamily(QLatin1String(DefaultFont));
    font.setPointSize(12);
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

QKeySequence Preferences::shortcut(const QString &actionId, const QKeySequence &fallback) const
{
    return m_shortcuts.value(actionId, fallback);
}

void Preferences::setShortcut(const QString &actionId, const QKeySequence &shortcut)
{
    m_shortcuts.insert(actionId, shortcut);
}

void Preferences::emitChanged()
{
    emit changed();
}
