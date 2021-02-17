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
    name = fpath.filename().c_str();
    ext = fpath.extension().c_str();
    if (name.isEmpty())
    {
        name = ext;
        ext.clear();
    }
    else
        name = name.left(name.count() - ext.count());

    if(isDirectory())
        ext = "<DIR>";

    if(ext[0] == '.')
        ext = ext.mid(1);

    icon = makeIcon();
}

FileSystemNode::~FileSystemNode()
{
    if(loader)
        loader->wait();
    loader = nullptr;
    qDeleteAll(children);
}

QIcon FileSystemNode::makeIcon() const
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

    if (!name.isEmpty())
    {
        QString n = name;
        n = n.remove('.');

        if (!n.isEmpty())
        {
            QString ch(n[0]);
            QFont f = painter.font();
            if(isDirectory())
                f.setBold(true);
            else
                f.setItalic(true);
            painter.setFont(f);

            painter.setPen(Qt::white);
            painter.drawText(QRect(0, 0, ICON_SIZE, ICON_SIZE), Qt::AlignCenter, ch); //ch.toUpper());
        }
    }
    painter.end();

    return pm;
}

Qt::ItemFlags FileSystemNode::flags() const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if(!isDirectory())
        f |= Qt::ItemNeverHasChildren;
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
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        if (column != 0)
            return {};

        if (role == Qt::DecorationRole)
            return icon;

        return {};
    }

    switch(column)
    {
    case COLUMN_NAME:
        return name;

    case COLUMN_EXT:
        return ext;

    case COLUMN_SIZE:
        if(isDirectory())
            return {};

        return (qlonglong)std::filesystem::file_size(fpath);
    }

    return {};
}

bool FileSystemNode::setData(int column, const QVariant &value)
{
    const QString s = value.toString();
    QString nname = name, next = ext;
    switch(column)
    {
    case COLUMN_NAME:
        if(s == name)
            return false;
        nname = s;
        break;

    case COLUMN_EXT:
        if(s == ext || isDirectory())
            return false;
        next = s;
        break;

    default:
        return false;
    }

    QString fname = nname;
    if (!next.isEmpty() && !isDirectory())
        fname += "." + next;

    path npath = fpath;
    npath.replace_filename(fname.toStdString());

    std::error_code ec;
    std::filesystem::rename(fpath, npath, ec);
    if(ec)
        return false;

    fpath = npath;
    name = nname;
    ext = next;
    icon = makeIcon();

    return true;
}

FileSystemNodeLoader* FileSystemNode::startLoad()
{
    if(!loader)
        loader = new FileSystemNodeLoader(this);
    return loader;
}

bool FileSystemNode::removeRows(int row, int count)
{
    if(row < 0 || row + count > children.count())
        return false;

    for(int i = row; i < row + count; ++i)
    {
        FileSystemNode* c = children[i];
        c->remove();
        delete c;
    }
    children.remove(row, count);
    return true;
}

void FileSystemNode::remove()
{
    // errors must be handled
    std::error_code ec;
    std::filesystem::remove_all(fpath, ec);
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
