// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "hotkeylistmodel.h"

#include "commands.h"

HotKeyListModel::HotKeyListModel(QObject *parent)
    : QAbstractItemModel{parent}
{
}

void HotKeyListModel::setUndoStack(QUndoStack *undo_stack_)
{
    undo_stack = undo_stack_;
}

auto HotKeyListModel::getUndoStack() const -> QUndoStack *
{
    return undo_stack;
}

auto HotKeyListModel::rowCount(const QModelIndex &parent) const -> int
{
    if(parent.isValid())
        return 0;

    return static_cast<int>(entries.count());
}

auto HotKeyListModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if(role != Qt::DisplayRole)
        return {};

    if(orientation != Qt::Horizontal)
        return {};

    if(section >= header.size())
        return {};

    return header.at(section);
}

auto HotKeyListModel::data(const QModelIndex &index, int role) const -> QVariant
{
    if(role != Qt::DisplayRole)
        return {};

    if(!index.isValid() || !checkIndex(index))
        return {};

    const auto &entry = entries.at(index.row());
    if(entry.is_error)
    {
        if(static_cast<Column>(index.column()) == Column::Keys)
            return {entry.error};

        return {};
    }

    switch(static_cast<Column>(index.column()))
    {
    case Column::BootOption:
        return entry.boot_option;

    case Column::Keys:
    {
        QVariant data;
        const auto &keys = entry.keys;
        data.setValue(&keys);
        return data;
    }

    case Column::VendorData:
        return {QString("%1B").arg(entry.vendor_data.size())};

    case Column::Count:
        return {};
    }

    Q_UNREACHABLE();
}

auto HotKeyListModel::setData(const QModelIndex &index, const QVariant &value, int role) -> bool
{
    if(role != Qt::EditRole)
        return false;

    if(!index.isValid() || !checkIndex(index))
        return false;

    auto row = index.row();
    if(const auto &entry = entries.at(row); entry.is_error)
        return false;

    QUndoCommand *command = nullptr;
    // Edited BootOption
    if(auto column = static_cast<Column>(index.column()); column == Column::BootOption && value.canConvert<uint16_t>() && index.data() != value)
        command = new SetHotKeyValueCommand<uint16_t>{*this, index, tr("boot option"), &HotKey::boot_option, value.value<uint16_t>()};

    // Edited keys
    else if(column == Column::Keys && value.canConvert<const EFIKeySequence *>() && *index.data().value<const EFIKeySequence *>() != *value.value<const EFIKeySequence *>())
        command = new SetHotKeyKeysCommand{*this, index, *value.value<const EFIKeySequence *>()};

    else
        return false;

    if(undo_stack)
        undo_stack->push(command);

    else
    {
        command->redo();
        delete command;
    }

    auto idx = this->index(row + 1, 0);
    Q_EMIT dataChanged(idx, idx, {role});
    return true;
}

auto HotKeyListModel::flags(const QModelIndex &index) const -> Qt::ItemFlags
{
    auto flags = QAbstractItemModel::flags(index);
    if(!index.isValid() || !checkIndex(index))
        return flags;

    if(const auto &entry = entries.at(index.row()); entry.is_error)
        return flags;

    if(static_cast<Column>(index.column()) == Column::VendorData)
        return flags;

    return flags | Qt::ItemIsEditable;
}

auto HotKeyListModel::insertRows(int row, int count, const QModelIndex &parent) -> bool
{
    for(int c = 0; c < count; ++c)
    {
        auto command = new InsertHotKeyCommand{*this, parent, row + c, {}};
        if(!undo_stack)
        {
            command->redo();
            delete command;
            continue;
        }

        undo_stack->push(command);
    }

    return true;
}

auto HotKeyListModel::appendRow(const HotKey &data, const QModelIndex &parent) -> bool
{
    // Only used internally when loading data, no undo/redo
    int row = rowCount(parent);
    beginInsertRows(parent, row, row);
    entries.append(data);
    endInsertRows();
    return true;
}

auto HotKeyListModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    for(int c = 0; c < count; ++c)
    {
        auto command = new RemoveHotKeyCommand{*this, parent, row};
        if(!undo_stack)
        {
            command->redo();
            delete command;
            continue;
        }

        undo_stack->push(command);
    }

    return true;
}

void HotKeyListModel::clear()
{
    beginResetModel();
    entries.clear();
    endResetModel();
}
