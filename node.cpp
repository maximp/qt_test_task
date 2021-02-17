#include "node.hpp"

#include <QPainter>
#include <QDebug>

FileSystemNode::FileSystemNode()
:   FileSystemNode(directory_entry(std::filesystem::current_path()))
{
    startLoad()->wait();
}

FileSystemNode::FileSystemNode(const QString &path)
:   FileSystemNode(directory_entry(std::filesystem::path(path.toStdString())))
{
    startLoad()->wait();
}

FileSystemNode::FileSystemNode(const directory_entry& e, FileSystemNode* p, int idx)
:   parentNode(p),
    parentIndex(idx),
    ftype(e.status().type()),
    fpath(e.path()),
    loaded(!isDirectory())
{
    enum {ICON_SIZE = 20};
    QPixmap pm(ICON_SIZE, ICON_SIZE);
    pm.fill(Qt::transparent);

    QPainter painter(&pm);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, ICON_SIZE, ICON_SIZE), 4, 4);
    QColor bgColor = isDirectory() ? QColor(0x21, 0x96, 0xf3) : QColor(0x4c, 0xaf, 0x50);
    painter.fillPath(path, bgColor);

    const std::string name = fpath.filename().string();
    if (!name.empty())
    {
        QFont f = painter.font();
        if(isDirectory())
            f.setBold(true);
        else
            f.setItalic(true);
        painter.setFont(f);

        QString ch(name[0]);
        painter.setPen(Qt::white);
        painter.drawText(QRect(0, 0, ICON_SIZE, ICON_SIZE), Qt::AlignCenter, ch.toUpper());
    }
    painter.end();

    icon = pm;
}

FileSystemNode::~FileSystemNode()
{
    if(loader)
        loader->wait();
    loader = nullptr;
    qDeleteAll(children);
}

Qt::ItemFlags FileSystemNode::flags() const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(!isDirectory())
        f |= Qt::ItemNeverHasChildren;
    else f |= Qt::ItemIsUserTristate;
    return f;
}

int FileSystemNode::count() const
{
    return !isLoaded() ? 1 : children.count();
}

int FileSystemNode::columnCount() const
{
    return COLUMN_COUNT;
}

FileSystemNode* FileSystemNode::child(int row) const
{
    if(row >= children.count())
        return nullptr;

    return children[row];
}

QVariant FileSystemNode::data(int column, int role) const
{
    if (role != Qt::DisplayRole)
    {
        if (column == 0 && role == Qt::DecorationRole)
            return icon;

        return {};
    }

    switch(column)
    {
    case COLUMN_NAME:
        return QString(fpath.filename().c_str());

    case COLUMN_EXT:
        if(isDirectory())
            return "<DIR>";
        return QString(fpath.extension().c_str());

    case COLUMN_SIZE:
        if(isDirectory())
            return {};

        return (qlonglong)std::filesystem::file_size(fpath);
    }

    return {};
}

FileSystemNodeLoader* FileSystemNode::startLoad()
{
    if(!loader)
        loader = new FileSystemNodeLoader(this);
    return loader;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileSystemNodeLoader::FileSystemNodeLoader(FileSystemNode* n)
:   node(n),
    thread(&FileSystemNodeLoader::run, this)
{
    qDebug() << "Start loading: " << node->fpath.c_str();
}

void FileSystemNodeLoader::run()
{
    namespace fs = std::filesystem;

    QVector<directory_entry> dirs, files;
    for(const fs::directory_entry& e: fs::directory_iterator(node->fpath))
    {
        const fs::path& p = e.path();
        (void)p;
        const fs::file_type t = e.status().type();
        switch(t)
        {
        case fs::file_type::regular:
            files.append(e);
            break;
        case fs::file_type::directory:
            dirs.append(e);
            break;
        default:
            break;
        }
    }

    entries.append(dirs);
    entries.append(files);

    int idx = 0;
    for(const fs::directory_entry& e: entries)
        children.append(new FileSystemNode(e, node, idx++));
}

void FileSystemNodeLoader::wait()
{
    thread.join();

    node->children = std::move(children);
    node->loaded = true;
    node->loader = nullptr;

    qDebug() << "Finish loading: " << node->fpath.c_str();

    delete this;
}
