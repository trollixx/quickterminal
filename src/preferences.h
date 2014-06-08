#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QFont>
#include <QKeySequence>
#include <QMap>
#include <QObject>

class QAction;

class Preferences : public QObject
{
    Q_OBJECT
public:
    static Preferences *instance();

    void load();
    void save();

    QFont defaultFont() const;

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

public slots:
    /// TODO: Remove in a new implementation
    void emitChanged();

signals:
    void changed();

private:
    static Preferences *m_instance;

    Preferences(QObject *parent = nullptr);
    Q_DISABLE_COPY(Preferences)
    ~Preferences() override;

    void migrate();
};

#endif // PREFERENCES_H
