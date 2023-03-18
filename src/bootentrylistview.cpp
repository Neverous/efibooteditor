// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrylistview.h"

BootEntryListView::BootEntryListView(QWidget *parent)
    : QListView{parent}
{
    setItemDelegate(&delegate);
}

void BootEntryListView::setModel(BootEntryListModel *model)
{
    readonly = model->readonly;
    delegate.setReadOnly(readonly);
    QListView::setModel(model);
}

void BootEntryListView::insertRow()
{
    if(readonly)
        return;

    auto row = currentIndex().row();
    model()->insertRow(row + 1);
    setCurrentIndex(model()->index(row + 1, 0));
}

void BootEntryListView::removeCurrentRow()
{
    if(readonly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    auto row = index.row();
    model()->removeRow(row);
    index = model()->index(row - 1, 0);
    if(!index.isValid())
        index = model()->index(row, 0);

    setCurrentIndex(index);
}

void BootEntryListView::moveCurrentRowUp()
{
    if(readonly)
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
    if(readonly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount() - 1)
        return;

    auto next_index = index.siblingAtRow(index.row() + 1);
    model()->moveRow(index, index.row(), next_index, next_index.row());
    setCurrentIndex(next_index);
}

void BootEntryListView::selectionChanged(const QItemSelection &selection, const QItemSelection &)
{
    QModelIndex index;
    if(!selection.indexes().isEmpty())
        index = selection.indexes().first();

    emit selected(index);
}
