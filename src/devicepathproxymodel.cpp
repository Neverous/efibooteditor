// SPDX-License-Identifier: LGPL-3.0-or-later
#include "devicepathproxymodel.h"

DevicePathProxyModel::DevicePathProxyModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void DevicePathProxyModel::setBootEntryListModel(BootEntryListModel &model)
{
    boot_entry_list_model = &model;
}

void DevicePathProxyModel::setBootEntryItem(const QModelIndex &index, const BootEntry *item)
{
    beginResetModel();
    boot_entry_index = index;
    boot_entry_device_path = !item ? nullptr : &item->device_path;
    endResetModel();
}

auto DevicePathProxyModel::rowCount(const QModelIndex &parent) const -> int
{
    if(parent.isValid())
        return 0;

    if(!boot_entry_device_path)
        return 0;

    return static_cast<int>(boot_entry_device_path->size());
}

auto DevicePathProxyModel::data(const QModelIndex &index, int) const -> QVariant
{
    if(!index.isValid() || !checkIndex(index))
        return {};

    QVariant data;
    data.setValue(&boot_entry_device_path->at(index.row()));
    return data;
}

auto DevicePathProxyModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    if(role != Qt::EditRole)
        return false;

    if(!index.isValid() || !checkIndex(index))
        return false;

    boot_entry_list_model->changeData(boot_entry_index, [&index, &value](BootEntry &entry)
        {
        entry.device_path[index.row()] = *value.value<const File_path::ANY *>();
        entry.formatDevicePath();
        return true; });

    emit dataChanged(index, index, {role});
    return true;
}

auto DevicePathProxyModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginInsertRows(parent, row, row + count - 1);
    boot_entry_list_model->changeData(boot_entry_index, [row, count](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.device_path.insert(static_cast<decltype(entry.device_path)::size_type>(row) + c, {});

        entry.formatDevicePath();
        return true; });
    endInsertRows();
    return true;
}

auto DevicePathProxyModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    beginRemoveRows(parent, row, row + count - 1);
    boot_entry_list_model->changeData(boot_entry_index, [row, count](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.device_path.removeAt(row);

        entry.formatDevicePath();
        return true; });
    endRemoveRows();
    return true;
}

auto DevicePathProxyModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) -> bool
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count, destinationParent, destinationChild);
    boot_entry_list_model->changeData(boot_entry_index, [sourceRow, count, destinationChild](BootEntry &entry)
        {
        for(int c = 0; c < count; ++c)
            entry.device_path.move(sourceRow, static_cast<decltype(entry.device_path)::size_type>(destinationChild) + (sourceRow < destinationChild ? 0 : c));

        entry.formatDevicePath();
        return true; });
    endMoveRows();
    return true;
}

void DevicePathProxyModel::clear()
{
    beginResetModel();
    boot_entry_list_model->changeData(boot_entry_index, [](BootEntry &entry)
        {
        entry.device_path.clear();
        entry.formatDevicePath();
        return true; });
    endResetModel();
}
