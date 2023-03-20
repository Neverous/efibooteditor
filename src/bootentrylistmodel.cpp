// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrylistmodel.h"
#include "commands.h"

BootEntryListModel::BootEntryListModel(const QString &name_, bool readonly_, QObject *parent)
    : QAbstractListModel{parent}
    , name{name_}
    , readonly{readonly_}
{
}

void BootEntryListModel::setUndoStack(QUndoStack *undo_stack_)
{
    undo_stack = undo_stack_;
}

auto BootEntryListModel::getUndoStack() const -> QUndoStack *
{
    return undo_stack;
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

void BootEntryListModel::setNextBootEntry(const QModelIndex &index, bool value)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    if(value)
    {
        if(next_boot == index)
            return;

        undo_stack->beginMacro(tr("Set Next boot to \"%1\"").arg(entries.at(index.row()).getTitle()));
        if(next_boot.isValid() && checkIndex(next_boot))
            setEntryNextBoot(next_boot, false);

        next_boot = index;
        setEntryNextBoot(next_boot, true);
        undo_stack->endMacro();
        return;
    }

    if(next_boot != index)
        return;

    setEntryNextBoot(next_boot, false);
    next_boot = QModelIndex{};
}

void BootEntryListModel::setEntryFilePath(const QModelIndex &index, int row, const File_path::ANY &file_path)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryFilePathCommand{*this, index, row, file_path};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::insertEntryFilePath(const QModelIndex &index, int row, const File_path::ANY &file_path)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new InsertBootEntryFilePathCommand{*this, index, row, file_path};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::removeEntryFilePath(const QModelIndex &index, int row)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new RemoveBootEntryFilePathCommand{*this, index, row};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::moveEntryFilePath(const QModelIndex &index, int source_row, int destination_row)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new MoveBootEntryFilePathCommand{*this, index, source_row, destination_row};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::clearEntryDevicePath(const QModelIndex &index)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    // Used only internally, no undo/redo
    entries[index.row()].device_path.clear();
    emit dataChanged(index, index, {Qt::EditRole});
}

void BootEntryListModel::setEntryIndex(const QModelIndex &index, uint16_t value)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryValueCommand<uint16_t>{*this, index, tr("Index"), &BootEntry::index, value};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::setEntryDescription(const QModelIndex &index, const QString &text)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryValueCommand<QString>{*this, index, tr("Description"), &BootEntry::description, text};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

bool BootEntryListModel::changeEntryOptionalDataFormat(const QModelIndex &index, int format)
{
    if(!index.isValid() || !checkIndex(index))
        return false;

    auto fmt = static_cast<BootEntry::OptionalDataFormat>(format);
    if(!entries[index.row()].changeOptionalDataFormat(fmt, true))
        return false;

    auto command = new ChangeOptionalDataFormatCommand{*this, index, fmt};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return true;
    }

    undo_stack->push(command);
    return true;
}

void BootEntryListModel::setEntryOptionalData(const QModelIndex &index, const QString &text)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryValueCommand<QString>{*this, index, tr("Optional data"), &BootEntry::optional_data, text};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::setEntryAttributes(const QModelIndex &index, uint32_t value)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryValueCommand<uint32_t>{*this, index, tr("Attributes"), &BootEntry::attributes, value};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void BootEntryListModel::setEntryNextBoot(const QModelIndex &index, bool value)
{
    if(!index.isValid() || !checkIndex(index))
        return;

    auto command = new SetBootEntryValueCommand<bool>{*this, index, tr("Next boot"), &BootEntry::is_next_boot, value};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
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
    for(int c = 0; c < count; ++c)
    {
        auto command = new InsertBootEntryCommand{*this, parent, row + c, {}};
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

auto BootEntryListModel::appendRow(const BootEntry &data, const QModelIndex &parent) -> bool
{
    // Only used internally when loading data, no undo/redo
    int row = rowCount(parent);
    beginInsertRows(parent, row, row);
    entries.append(data);
    endInsertRows();
    return true;
}

auto BootEntryListModel::removeRows(int row, int count, const QModelIndex &parent) -> bool
{
    for(int c = 0; c < count; ++c)
    {
        auto command = new RemoveBootEntryCommand{*this, parent, row};
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

auto BootEntryListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) -> bool
{
    for(int c = 0; c < count; ++c)
    {
        auto command = new MoveBootEntryCommand{*this, sourceParent, sourceRow, destinationParent, destinationChild + (sourceRow < destinationChild ? 0 : c)};
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

void BootEntryListModel::clear()
{
    beginResetModel();
    entries.clear();
    endResetModel();
}
