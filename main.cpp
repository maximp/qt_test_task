#include <QApplication>
#include <QTreeView>
#include <QScreen>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>

#include "model.hpp"

class Tree : public QTreeView
{
public:
    Tree(QWidget* parent = nullptr);

protected:
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    FileSystemModel model;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow main;

    QWidget central;
    QVBoxLayout* vbox = new QVBoxLayout(&central);

    QLineEdit filter;
    vbox->addWidget(&filter);

    Tree view;
    vbox->addWidget(&view);

    main.setCentralWidget(&central);

    main.show();

    return a.exec();
}

Tree::Tree(QWidget* parent)
:   QTreeView(parent)
{
    setModel(&model);

    for (int column = 0; column < model.columnCount(); ++column)
        resizeColumnToContents(column);

    setEditTriggers(EditTrigger::DoubleClicked);
}

void Tree::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeView::selectionChanged(selected, deselected);
    for(const auto& m : selected.indexes())
        model.load(m);
}