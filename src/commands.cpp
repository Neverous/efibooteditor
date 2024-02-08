// SPDX-License-Identifier: LGPL-3.0-or-later

#include "commands.h"

InsertRemoveBootEntryCommand::InsertRemoveBootEntryCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent)
    : QUndoCommand(description, parent)
    , model{model_}
    , index_parent{index_parent_}
    , entry{entry_}
    , index{index_}
{
}

int InsertRemoveBootEntryCommand::id() const
{
    return -1;
}

void InsertRemoveBootEntryCommand::insert()
{
    model.beginInsertRows(index_parent, index, index);
    model.entries.insert(index, entry);
    model.endInsertRows();
}

void InsertRemoveBootEntryCommand::remove()
{
    model.beginRemoveRows(index_parent, index, index);
    model.entries.removeAt(index);
    model.endRemoveRows();
}

InsertBootEntryCommand::InsertBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent)
    : InsertRemoveBootEntryCommand(model_, QObject::tr("Insert %1 entry \"%2\" at position %3").arg(model_.name, entry_.getTitle()).arg(index_), index_parent_, index_, entry_, parent)
{
}

void InsertBootEntryCommand::undo()
{
    remove();
}

void InsertBootEntryCommand::redo()
{
    insert();
}

RemoveBootEntryCommand::RemoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, QUndoCommand *parent)
    : InsertRemoveBootEntryCommand(model_, QObject::tr("Removing %1 entry \"%2\" from position %3").arg(model_.name, model_.entries.at(index_).getTitle()).arg(index_), index_parent_, index_, model_.entries.at(index_), parent)
{
}

void RemoveBootEntryCommand::undo()
{
    insert();
}

void RemoveBootEntryCommand::redo()
{
    remove();
}

MoveBootEntryCommand::MoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &source_parent_, int source_index_, const QModelIndex &destination_parent_, int destination_index_, QUndoCommand *parent)
    : QUndoCommand("", parent)
    , model{model_}
    , title{model_.entries.at(source_index_).getTitle()}
    , source_parent{source_parent_}
    , destination_parent{destination_parent_}
    , source_index{source_index_}
    , destination_index{destination_index_}
{
    setText(QObject::tr("Move %1 entry \"%2\" from position %3 to %4").arg(model.name, title).arg(source_index).arg(destination_index));
}

int MoveBootEntryCommand::id() const
{
    return 2;
}

void MoveBootEntryCommand::undo()
{
    model.beginMoveRows(destination_parent, destination_index, destination_index + 1, source_parent, source_index);
    model.entries.move(destination_index, source_index);
    model.endMoveRows();
}

void MoveBootEntryCommand::redo()
{
    model.beginMoveRows(source_parent, source_index, source_index + 1, destination_parent, destination_index);
    model.entries.move(source_index, destination_index);
    model.endMoveRows();
}

bool MoveBootEntryCommand::mergeWith(const QUndoCommand *command)
{
    if(command->id() != id())
        return false;

    auto cmd = static_cast<const MoveBootEntryCommand *>(command);
    if(&cmd->model != &model)
        return false;

    if(cmd->source_parent != destination_parent)
        return false;

    if(cmd->source_index != destination_index)
        return false;

    destination_parent = cmd->destination_parent;
    destination_index = cmd->destination_index;
    setText(QObject::tr("Move %1 entry \"%2\" from position %3 to %4").arg(model.name, title).arg(source_index).arg(destination_index));
    if(source_index == destination_index)
        setObsolete(true);

    return true;
}

ChangeOptionalDataFormatCommand::ChangeOptionalDataFormatCommand(BootEntryListModel &model_, const QModelIndex &index_, const BootEntry::OptionalDataFormat &value_, QUndoCommand *parent)
    : QUndoCommand{"", parent}
    , model{model_}
    , title{model_.entries.at(index_.row()).getTitle()}
    , index{index_}
    , value{value_}
{
    updateTitle(value);
}

int ChangeOptionalDataFormatCommand::id() const
{
    return 4;
}

void ChangeOptionalDataFormatCommand::undo()
{
    redo();
}

void ChangeOptionalDataFormatCommand::redo()
{
    auto &entry = model.entries[index.row()];
    auto old_value = entry.optional_data_format;
    entry.changeOptionalDataFormat(value);
    value = old_value;
    emit model.dataChanged(index, index, {Qt::EditRole});
}

bool ChangeOptionalDataFormatCommand::mergeWith(const QUndoCommand *command)
{
    auto cmd = static_cast<const ChangeOptionalDataFormatCommand *>(command);
    if(&cmd->model != &model)
        return false;

    if(cmd->index != index)
        return false;

    const auto &entry = model.entries.at(index.row());
    if(value == entry.optional_data_format)
        setObsolete(true);

    updateTitle(entry.optional_data_format);
    return true;
}

void ChangeOptionalDataFormatCommand::updateTitle(BootEntry::OptionalDataFormat val)
{
    QString format{};
    switch(val)
    {
    case BootEntry::OptionalDataFormat::Base64:
        format = "Base64";
        break;

    case BootEntry::OptionalDataFormat::Hex:
        format = "Hex";
        break;

    case BootEntry::OptionalDataFormat::Utf16:
        format = "UTF-16";
        break;

    case BootEntry::OptionalDataFormat::Utf8:
        format = "UTF-8";
        break;
    }

    setText(QObject::tr("Change %1 entry \"%2\" %3 to \"%4\"").arg(model.name, title, QObject::tr("Optional data"), format));
}

InsertRemoveBootEntryFilePathCommand::InsertRemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_, int row_, const FilePath::ANY &file_path_, QUndoCommand *parent)
    : QUndoCommand(description, parent)
    , model{model_}
    , file_path{file_path_}
    , index{index_}
    , row{row_}
{
}

int InsertRemoveBootEntryFilePathCommand::id() const
{
    return -1;
}

void InsertRemoveBootEntryFilePathCommand::insert()
{
    auto &entry = model.entries[index.row()];
    entry.device_path.insert(row, file_path);
    entry.formatDevicePath();
    emit model.dataChanged(index, index, {Qt::EditRole});
}

void InsertRemoveBootEntryFilePathCommand::remove()
{
    auto &entry = model.entries[index.row()];
    entry.device_path.removeAt(row);
    entry.formatDevicePath();
    emit model.dataChanged(index, index, {Qt::EditRole});
}

InsertBootEntryFilePathCommand::InsertBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const FilePath::ANY &file_path_, QUndoCommand *parent)
    : InsertRemoveBootEntryFilePathCommand(model_, QObject::tr("Insert %1 entry \"%2\" file path at position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), index_, row_, file_path_, parent)
{
}

void InsertBootEntryFilePathCommand::undo()
{
    remove();
}

void InsertBootEntryFilePathCommand::redo()
{
    insert();
}

RemoveBootEntryFilePathCommand::RemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, QUndoCommand *parent)
    : InsertRemoveBootEntryFilePathCommand(model_, QObject::tr("Removing %1 entry \"%2\" file path from position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), index_, row_, model_.entries.at(index_.row()).device_path.at(row_), parent)
{
}

void RemoveBootEntryFilePathCommand::undo()
{
    insert();
}

void RemoveBootEntryFilePathCommand::redo()
{
    remove();
}

SetBootEntryFilePathCommand::SetBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const FilePath::ANY &value_, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("Setting %1 entry \"%2\" file path at position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), parent)
    , model{model_}
    , index{index_}
    , value{value_}
    , row{row_}
{
}

int SetBootEntryFilePathCommand::id() const
{
    return -1;
}

void SetBootEntryFilePathCommand::undo()
{
    redo();
}

void SetBootEntryFilePathCommand::redo()
{
    auto &entry = model.entries[index.row()];
    auto old_value = entry.device_path[row];
    entry.device_path[row] = value;
    value = old_value;
    entry.formatDevicePath();
    emit model.dataChanged(index, index, {Qt::EditRole});
}

MoveBootEntryFilePathCommand::MoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int source_row_, int destination_row_, QUndoCommand *parent)
    : QUndoCommand("", parent)
    , model{model_}
    , title{model_.entries.at(index_.row()).getTitle()}
    , index{index_}
    , source_row{source_row_}
    , destination_row{destination_row_}
{
    setText(QObject::tr("Move %1 entry \"%2\" file path from position %3 to %4").arg(model.name, title).arg(source_row).arg(destination_row));
}

int MoveBootEntryFilePathCommand::id() const
{
    return 5;
}

void MoveBootEntryFilePathCommand::undo()
{
    auto &entry = model.entries[index.row()];
    entry.device_path.move(destination_row, source_row);
    entry.formatDevicePath();
    emit model.dataChanged(index, index, {Qt::EditRole});
}

void MoveBootEntryFilePathCommand::redo()
{
    auto &entry = model.entries[index.row()];
    entry.device_path.move(source_row, destination_row);
    entry.formatDevicePath();
    emit model.dataChanged(index, index, {Qt::EditRole});
}

bool MoveBootEntryFilePathCommand::mergeWith(const QUndoCommand *command)
{
    if(command->id() != id())
        return false;

    auto cmd = static_cast<const MoveBootEntryFilePathCommand *>(command);
    if(&cmd->model != &model)
        return false;

    if(cmd->index != index)
        return false;

    if(cmd->source_row != destination_row)
        return false;

    destination_row = cmd->destination_row;
    setText(QObject::tr("Move %1 entry \"%2\" file path from position %3 to %4").arg(model.name, title).arg(source_row).arg(destination_row));
    if(source_row == destination_row)
        setObsolete(true);

    return true;
}
