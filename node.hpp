#pragma once

#include <QAbstractItemModel>
#include <QIcon>

#include <atomic>
#include <filesystem>
#include <thread>

class FileSystemNodeLoader;

class FileSystemNode
{
public:

    enum {
        COLUMN_NAME,
        COLUMN_EXT,
        COLUMN_SIZE,
        COLUMN_COUNT
    };

    FileSystemNode();
    FileSystemNode(const QString &path);
    ~FileSystemNode();

    Qt::ItemFlags flags() const;
    int count() const;
    int columnCount() const;
    FileSystemNode* child(int row) const;
    FileSystemNode* parent() const { return parentNode; }
    int row() const { return parentIndex; }
    QVariant data(int column, int role) const;
    bool setData(int column, const QVariant &value);

    bool isDirectory() const { return ftype == file_type::directory; }
    bool isLoaded() const { return loaded; }

    FileSystemNodeLoader* startLoad();

    bool removeRows(int row, int count);

private:
    using file_type = std::filesystem::file_type;
    using directory_entry = std::filesystem::directory_entry;
    using path = std::filesystem::path;

    FileSystemNode*          parentNode = nullptr;
    int                      parentIndex = 0;
    QVector<FileSystemNode*> children;
    file_type                ftype = file_type::none;
    path                     fpath;
    bool                     loaded;
    QIcon                    icon;
    FileSystemNodeLoader*    loader = nullptr;
    QString                  name;
    QString                  ext;

    FileSystemNode(const directory_entry& e, FileSystemNode* p = nullptr, int idx = 0);

    QIcon makeIcon() const;
    void remove();

    friend class FileSystemNodeLoader;
};

class FileSystemNodeLoader
{
public:
    FileSystemNodeLoader(FileSystemNode* n);

    void wait();

private:
    using directory_entry = std::filesystem::directory_entry;

    FileSystemNode*          node;
    std::thread              thread;
    QVector<directory_entry> entries;
    QVector<FileSystemNode*> children;

    void run();
};