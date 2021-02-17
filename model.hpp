#pragma once

#include <QAbstractItemModel>

class FileSystemNode;

class FileSystemModel : public QAbstractItemModel
{
public:
    explicit FileSystemModel(QObject *parent = nullptr);
    explicit FileSystemModel(const QString &path, QObject *parent = nullptr);
    ~FileSystemModel() override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const override;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool canFetchMore(const QModelIndex &parent) const override;
    virtual void fetchMore(const QModelIndex &parent) override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void load(const QModelIndex& parent);

private:
    FileSystemNode* root = nullptr;

    static FileSystemNode* node(const QModelIndex& index);
};