// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "hotkeysview.h"

HotKeysView::HotKeysView(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegateForColumn(0, &bootOptionDelegate);
    setItemDelegateForColumn(1, &keysDelegate);

    connect(this, SIGNAL(clicked(QModelIndex)), SLOT(edit(QModelIndex)));
}

void HotKeysView::refreshBootOptions(const BootEntryListModel &model)
{
    bootOptionDelegate.refreshBootOptions(model);
}

void HotKeysView::setMaxKeyCount(qsizetype keys)
{
    keysDelegate.setMaximumSequenceLength(keys);
}

void HotKeysView::insertRow()
{
    auto index = currentIndex();
    auto row = index.row();
    model()->insertRow(row + 1);
    index = model()->index(row + 1, 0);
    setCurrentIndex(index);
}

void HotKeysView::removeCurrentRow()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    auto row = index.row();
    model()->removeRow(row);
}

void HotKeysView::setFilter(const QString &filter)
{
    for(int row = 0; row < model()->rowCount(); ++row)
        setRowHidden(row, {}, !toHex(model()->data(model()->index(row, 0)).value<uint16_t>(), 4).startsWith(filter));
}
