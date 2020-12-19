// SPDX-License-Identifier: LGPL-3.0-or-later
#include "include/bootentrylistmodel.h"

BootEntryListModel::BootEntryListModel(QObject *parent)
    : QAbstractListModel{parent}
{
}

auto BootEntryListModel::rowCount(const QModelIndex &parent) const -> int
{
    if(parent.isValid())
        return 0;

    return static_cast<int>(entries.count());
}

auto BootEntryListModel::data(const QModelIndex &index, int) const -> QVariant
{
    if(!index.isValid() || !checkIndex(index))
        return {};

    QVariant data;
    data.setValue(&entries.at(index.row()));
    return data;
}

auto BootEntryListModel::changeData(const QModelIndex &index, const Change_fn &change_fn, int role) -> bool
{
    if(role != Qt::EditRole)
        return false;

    if(!index.isValid() || !checkIndex(index))
        return false;

    if(!change_fn(entries[index.row()]))
        return false;

    emit dataChanged(index, index, {role});
    return true;
}

void BootEntryListModel::setNextBootEntry(const QModelIndex &index, bool value)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    if(value)
    {
        if(next_boot == index)
            return;

        if(next_boot.isValid() && checkIndex(next_boot))
            changeData(next_boot, [](BootEntry &entry) {
                entry.is_next_boot = false;
                return true;
            });

        next_boot = index;
        changeData(next_boot, [](BootEntry &entry) {
            entry.is_next_boot = true;
            return true;
        });

        return;
    }

    if(next_boot != index)
        return;

    changeData(next_boot, [](BootEntry &entry) {
        entry.is_next_boot = false;
        return true;
    });

    next_boot = QModelIndex{};
}

auto BootEntryListModel::flags(const QModelIndex &index) const -> Qt::ItemFlags
{
    auto flags = QAbstractListModel::flags(index);
    if(index.isValid() && checkIndex(index))
        return flags | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

    return flags;
}

auto BootEntryListModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginInsertRows(parent, row, row + count - 1);
    for(int c = 0; c < count; ++c)
        entries.insert(row + c, {});

    endInsertRows();
    return true;
}

auto BootEntryListModel::appendRow(const BootEntry &data, const QModelIndex &parent) -> bool
{
    int row = rowCount(parent);
    beginInsertRows(parent, row, row);
    entries.append(data);
    endInsertRows();
    return true;
}

auto BootEntryListModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginRemoveRows(parent, row, row + count - 1);
    for(int c = 0; c < count; ++c)
        entries.removeAt(row);

    endRemoveRows();
    return true;
}

auto BootEntryListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) -> bool
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    for(int c = 0; c < count; ++c)
        entries.move(sourceRow, destinationChild + (sourceRow < destinationChild ? 0 : c));

    endMoveRows();
    return true;
}

void BootEntryListModel::clear()
{
    beginResetModel();
    entries.clear();
    endResetModel();
}
