#include "model.hpp"
#include "node.hpp"

FileSystemModel::FileSystemModel(QObject *parent)
:   QAbstractItemModel(parent),
    root(new FileSystemNode())
{}

FileSystemModel::FileSystemModel(const QString &path, QObject *parent)
:   QAbstractItemModel(parent),
    root(new FileSystemNode(path))
{}

FileSystemModel::~FileSystemModel()
{
    delete root;
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    FileSystemNode* n = node(index);
    Qt::ItemFlags f = n->flags() | QAbstractItemModel::flags(index);

    if(index.column() == FileSystemNode::COLUMN_NAME)
        f |= Qt::ItemIsEditable;

    if(index.column() == FileSystemNode::COLUMN_EXT && !n->isDirectory())
        f |= Qt::ItemIsEditable;

    return f;
}

QVariant FileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch(section)
    {
    case FileSystemNode::COLUMN_NAME: return "Name";
    case FileSystemNode::COLUMN_EXT: return "Ext";
    case FileSystemNode::COLUMN_SIZE: return "Size";
    default: return {};
    }
}

QModelIndex FileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    FileSystemNode* parentNode = parent.isValid() ? node(parent) : root;
    FileSystemNode* childNode = parentNode->child(row);
    if (childNode)
        return createIndex(row, column, childNode);

    return QModelIndex();
}

QModelIndex FileSystemModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return {};

    FileSystemNode* childNode = node(child);
    FileSystemNode* parentNode = childNode->parent();

    if (parentNode == root)
        return {};

    return createIndex(parentNode->row(), 0, parentNode);
}

int FileSystemModel::rowCount(const QModelIndex &parent) const
{
    FileSystemNode* n = parent.isValid() ? node(parent) : root;
    return n->count();
}

int FileSystemModel::columnCount(const QModelIndex &parent) const
{
    FileSystemNode* n = parent.isValid() ? node(parent) : root;
    return n->columnCount();
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    FileSystemNode* n = node(index);

    return n->data(index.column(), role);
}

bool FileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    FileSystemNode* n = node(index);
    bool result = n->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool FileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return false;

    FileSystemNode* n = node(parent);

    return !n->isLoaded();
}

void FileSystemModel::fetchMore(const QModelIndex &parent)
{
    if (!parent.isValid())
        return;

    FileSystemNode* n = node(parent);
    FileSystemNodeLoader* loader = n->startLoad();
    loader->wait();

    // this code damages view layout when using with QSortFilterProxyModel
    // at some cases
    //
    //beginInsertRows(parent, 0, n->count() - 1);
    //endInsertRows();
}

bool FileSystemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(count <= 0)
        return false;

    FileSystemNode* n = parent.isValid() ? node(parent) : root;

    beginRemoveRows(parent, row, row + count - 1);
    bool result = n->removeRows(row, count);
    endRemoveRows();

    return result;
}

void FileSystemModel::load(const QModelIndex& parent)
{
    if (!parent.isValid())
        return;

    FileSystemNode* n = node(parent);

    if(!n->isLoaded())
        n->startLoad();
}

FileSystemNode* FileSystemModel::node(const QModelIndex& index)
{
    return reinterpret_cast<FileSystemNode*>(index.internalPointer());
}
