#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class QMainWindow;
class QxtGlobalShortcut;

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);

signals:

public slots:
    void createWindow();

private slots:
    void preferencesChanged();

private:
    void parseOptions();
    void setDropShortcut(const QKeySequence &shortcut);

    QList<QMainWindow *> m_windows;

    // Command line options
    QString m_command;
    bool m_dropMode = false;
    QString m_workingDir;

    QxtGlobalShortcut *m_dropShortcut = nullptr;
};

#endif // APPLICATION_H
