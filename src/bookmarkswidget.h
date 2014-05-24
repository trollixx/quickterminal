#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H

#include "ui_bookmarkswidget.h"

class AbstractBookmarkItem;
class BookmarksModel;

class BookmarksWidget : public QWidget, Ui::BookmarksWidget
{
    Q_OBJECT
public:
    explicit BookmarksWidget(QWidget *parent = nullptr);

    void setup();

signals:
    void callCommand(const QString &cmd);

private slots:
    void handleCommand(const QModelIndex &index);

private:
    BookmarksModel *m_model;
};

class BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit BookmarksModel(QObject *parent = nullptr);
    ~BookmarksModel();

    void setup();

    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    AbstractBookmarkItem *getItem(const QModelIndex &index) const;
    AbstractBookmarkItem *m_root = nullptr;
};

#endif // BOOKMARKSWIDGET_H
