#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QFont>
#include <QKeySequence>
#include <QMap>
#include <QObject>

class QAction;
class QSettings;

class Preferences : public QObject
{
    Q_OBJECT
public:
    static Preferences *instance();

    void load();
    void save();

    bool hasShortcut(const QString &actionId) const;
    QKeySequence shortcut(const QString &actionId,
                          const QKeySequence &fallback = QKeySequence()) const;
    void setShortcut(const QString &actionId, const QKeySequence &shortcut);
    void removeShortcut(const QString &actionId);
    QStringList shortcutActions() const;

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

    int appOpacity;
    int termOpacity;

    int scrollBarPos;
    int tabsPos;
    bool alwaysShowTabs;
    int m_motionAfterPaste;

    bool tabBarless;
    bool menuVisible;

    bool askOnExit;

    bool useCWD;

    bool dropKeepOpen;
    bool dropShowOnStart;
    int dropWidht;
    int dropHeight;

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

    QSettings *m_settings = nullptr;
    QMap<QString, QKeySequence> m_shortcuts;
};

#endif // PREFERENCES_H
