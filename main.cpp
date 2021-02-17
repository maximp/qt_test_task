#include <QApplication>
#include <QTreeView>
#include <QScreen>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QKeyEvent>
#include <QSortFilterProxyModel>

#include "model.hpp"

class Tree : public QTreeView
{
public:
    Tree(QLineEdit* filter, QWidget* parent = nullptr);

protected:
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private:
    FileSystemModel fsModel;
    QSortFilterProxyModel filterModel;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow main;

    QWidget central;
    QVBoxLayout* vbox = new QVBoxLayout(&central);

    QLineEdit filter;
    vbox->addWidget(&filter);

    Tree view(&filter);
    vbox->addWidget(&view);

    main.setCentralWidget(&central);

    main.show();

    return a.exec();
}

Tree::Tree(QLineEdit* filter, QWidget* parent)
:   QTreeView(parent)
{
    filterModel.setSourceModel(&fsModel);
    setModel(&filterModel);

    QObject::connect(filter, &QLineEdit::textChanged,
        &filterModel, &QSortFilterProxyModel::setFilterWildcard);

    for (int column = 0; column < fsModel.columnCount(); ++column)
        resizeColumnToContents(column);

    setEditTriggers(EditTrigger::DoubleClicked);
}

void Tree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    for(const auto& m : selected.indexes())
        fsModel.load(m);
}

void Tree::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);
    if(event->key() == Qt::Key_Delete)
    {
        const QModelIndex index = selectionModel()->currentIndex();
        QAbstractItemModel* m = model();
        m->removeRow(index.row(), index.parent());
    }
}