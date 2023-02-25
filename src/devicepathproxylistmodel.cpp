// SPDX-License-Identifier: LGPL-3.0-or-later
#include "devicepathproxylistmodel.h"

DevicePathProxyListModel::DevicePathProxyListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void DevicePathProxyListModel::setBootEntryListModel(BootEntryListModel &model)
{
    boot_entry_list_model = &model;
}

void DevicePathProxyListModel::setBootEntryItem(const QModelIndex &index, const BootEntry *item)
{
    beginResetModel();
    boot_entry_index = index;
    boot_entry_file_path = !item ? nullptr : &item->file_path;
    endResetModel();
}

auto DevicePathProxyListModel::rowCount(const QModelIndex &parent) const -> int
{
    if(parent.isValid())
        return 0;

    if(!boot_entry_file_path)
        return 0;

    return static_cast<int>(boot_entry_file_path->size());
}

auto DevicePathProxyListModel::data(const QModelIndex &index, int) const -> QVariant
{
    if(!index.isValid() || !checkIndex(index))
        return {};

    QVariant data;
    data.setValue(&boot_entry_file_path->at(index.row()));
    return data;
}

auto DevicePathProxyListModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    if(role != Qt::EditRole)
        return false;

    if(!index.isValid() || !checkIndex(index))
        return false;

    boot_entry_list_model->changeData(boot_entry_index, [&index, &value](BootEntry &entry)
        {
        entry.file_path[index.row()] = *value.value<const Device_path::ANY *>();
        entry.formatFilePath();
        return true; });

    emit dataChanged(index, index, {role});
    return true;
}

auto DevicePathProxyListModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginInsertRows(parent, row, row + count - 1);
    boot_entry_list_model->changeData(boot_entry_index, [row, count](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.file_path.insert(static_cast<decltype(entry.file_path)::size_type>(row) + c, {});

        entry.formatFilePath();
        return true; });
    endInsertRows();
    return true;
}

auto DevicePathProxyListModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginRemoveRows(parent, row, row + count - 1);
    boot_entry_list_model->changeData(boot_entry_index, [row, count](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.file_path.removeAt(row);

        entry.formatFilePath();
        return true; });
    endRemoveRows();
    return true;
}

auto DevicePathProxyListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) -> bool
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    boot_entry_list_model->changeData(boot_entry_index, [sourceRow, count, destinationChild](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.file_path.move(sourceRow, static_cast<decltype(entry.file_path)::size_type>(destinationChild) + (sourceRow < destinationChild ? 0 : c));

        entry.formatFilePath();
        return true; });
    endMoveRows();
    return true;
}

void DevicePathProxyListModel::clear()
{
    beginResetModel();
    boot_entry_list_model->changeData(boot_entry_index, [](BootEntry &entry)
        {
        entry.file_path.clear();
        entry.formatFilePath();
        return true; });
    endResetModel();
}
