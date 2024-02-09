// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrylistview.h"

BootEntryListView::BootEntryListView(QWidget *parent)
    : QListView{parent}
{
    setItemDelegate(&delegate);
}

void BootEntryListView::setModel(BootEntryListModel *model)
{
    options = model->options;
    delegate.setOptions(options);
    if(this->model())
    {
        disconnect(this->model(), &BootEntryListModel::rowsMoved, this, &BootEntryListView::rowsMoved);
        disconnect(this->model(), &BootEntryListModel::rowsRemoved, this, &BootEntryListView::rowsChanged);
        disconnect(this->model(), &BootEntryListModel::rowsInserted, this, &BootEntryListView::rowsChanged);
    }

    QListView::setModel(model);
    connect(this->model(), &BootEntryListModel::rowsMoved, this, &BootEntryListView::rowsMoved);
    connect(this->model(), &BootEntryListModel::rowsRemoved, this, &BootEntryListView::rowsChanged);
    connect(this->model(), &BootEntryListModel::rowsInserted, this, &BootEntryListView::rowsChanged);
}

void BootEntryListView::insertRow()
{
    if(options & BootEntryListModel::Option::ReadOnly)
        return;

    auto row = currentIndex().row();
    model()->insertRow(row + 1);
    setCurrentIndex(model()->index(row + 1, 0));
}

void BootEntryListView::duplicateRow()
{
    if(options & BootEntryListModel::Option::ReadOnly)
        return;

    auto row = currentIndex().row();
    auto idx = model()->index(row, 0);
    model()->setData(idx, idx);
    setCurrentIndex(model()->index(row + 1, 0));
}

void BootEntryListView::removeCurrentRow()
{
    if(options & BootEntryListModel::Option::ReadOnly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    auto row = index.row();
    model()->removeRow(row);
}

void BootEntryListView::moveCurrentRowUp()
{
    if(options & BootEntryListModel::Option::ReadOnly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() == 0)
        return;

    auto previous_index = index.siblingAtRow(index.row() - 1);
    model()->moveRow(index, index.row(), previous_index, previous_index.row());
    setCurrentIndex(previous_index);
}

void BootEntryListView::moveCurrentRowDown()
{
    if(options & BootEntryListModel::Option::ReadOnly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount() - 1)
        return;

    auto next_index = index.siblingAtRow(index.row() + 1);
    model()->moveRow(index, index.row(), next_index, next_index.row());
    setCurrentIndex(next_index);
}

void BootEntryListView::rowsMoved(const QModelIndex &, int sourceStart, int sourceEnd, const QModelIndex &, int destinationRow)
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount())
        return;

    if(sourceStart <= index.row() && index.row() <= sourceEnd)
        index = model()->index(index.row() + destinationRow - sourceStart, 0);

    else if(sourceStart <= destinationRow)
    {
        if(sourceEnd < index.row() && index.row() < destinationRow)
            index = model()->index(index.row() - sourceEnd + sourceStart, 0);
    }
    else
    {
        if(destinationRow < index.row() && index.row() < sourceStart)
            index = model()->index(index.row() + sourceEnd - sourceStart, 0);
    }

    setCurrentIndex(index);
}

void BootEntryListView::rowsChanged(const QModelIndex &, int, int)
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount())
        return;

    emit selected(index);
}

void BootEntryListView::selectionChanged(const QItemSelection &selection, const QItemSelection &)
{
    QModelIndex index;
    if(!selection.indexes().isEmpty())
        index = selection.indexes().first();

    emit selected(index);
}
