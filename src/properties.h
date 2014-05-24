#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QFont>
#include <QKeySequence>
#include <QMap>

class QAction;

class Properties
{
public:
    static Properties *Instance();

    QFont defaultFont() const;
    void saveSettings();

    QByteArray mainWindowGeometry;
    QByteArray mainWindowState;
    // ShortcutMap shortcuts;
    QString shell;
    QFont font;
    QString colorScheme;
    QString guiStyle;
    bool highlightCurrentTerminal;

    bool historyLimited;
    unsigned historyLimitedTo;

    QString emulation;

    QMap<QString, QString> sessions;

    int appOpacity;
    int termOpacity;

    int scrollBarPos;
    int tabsPos;
    bool alwaysShowTabs;
    int m_motionAfterPaste;

    bool borderless;
    bool tabBarless;
    bool menuVisible;

    bool askOnExit;

    bool useCWD;

    bool useBookmarks;
    bool bookmarksVisible;
    QString bookmarksFile;

    QKeySequence dropShortCut;
    bool dropKeepOpen;
    bool dropShowOnStart;
    int dropWidht;
    int dropHeight;

    QMap<QString, QAction *> actions;

    void loadSettings();

private:
    // Singleton handling
    static Properties *m_instance;

    Properties();
    Properties(const Properties &)
    {
    }

    ~Properties();
};

#endif // PROPERTIES_H
