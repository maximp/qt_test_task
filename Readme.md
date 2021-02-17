Qt test task
============

Write an application that displays the file system structure using QTreeView.

QTreeView widget should have 3 columns:
- Element name
- File extension (for the folder the constant '&lt;DIR&gt;')
- File size (empty for the folder)

Element name should be accompanied by an icon in the form of a square, filled with a single letter.
The first letter of the element name is used as the letter.

For a folder, the fill color is blue, for a file - green. The color of the letter is white.

The folder name should be displayed in bold, and the file name in italics.

Renaming an element starts with a double click.

By pressing the Del key on the keyboard - selected items are deleted (as well as from the file system).

The file system scan should be delayed (the contents of the folder are read when this folder is selected) in a separate thread.

The model must be inherited from the QAbstractItemModel.

An input field must be provided to filter the display by element name.

![Screenshot](https://github.com/maximp/qt_test_task/blob/master/screenshot.png?raw=true)

Screen with filtering:
![Screenshot](https://github.com/maximp/qt_test_task/blob/master/screenshot2.png?raw=true)
