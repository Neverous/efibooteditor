// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "efibootdata.h"
#include <QUndoCommand>

template <class Type>
class SetEFIBootDataValueCommand: public QUndoCommand
{
    typedef Type EFIBootData::*PropertyPtr;
    typedef void (EFIBootData::*SignalPtr)(const Type &);

    EFIBootData &data;
    const QString name;
    PropertyPtr property;
    SignalPtr signal;
    Type value;

public:
    SetEFIBootDataValueCommand(EFIBootData &data_, const QString &name_, PropertyPtr property_, SignalPtr signal_, const Type &value_, QUndoCommand *parent = nullptr)
        : QUndoCommand(QObject::tr("Change %1 to \"%2\"").arg(name_).arg(value_), parent)
        , data{data_}
        , name{name_}
        , property{property_}
        , signal{signal_}
        , value{value_}
    {
    }

    SetEFIBootDataValueCommand(const SetEFIBootDataValueCommand &) = delete;
    SetEFIBootDataValueCommand &operator=(const SetEFIBootDataValueCommand &) = delete;

    int id() const override
    {
        return 1;
    }

    void undo() override
    {
        redo();
    }

    void redo() override
    {
        auto old_value = data.*property;
        data.*property = value;
        value = old_value;
        emit(data.*signal)(data.*property);
    }

    bool mergeWith(const QUndoCommand *command) override
    {
        auto cmd = static_cast<const SetEFIBootDataValueCommand<Type> *>(command);
        if(&cmd->data != &data)
            return false;

        if(cmd->property != property)
            return false;

        if(cmd->signal != signal)
            return false;

        if(value == data.*property)
            setObsolete(true);

        setText(QObject::tr("Change %1 to \"%2\"").arg(name).arg(data.*property));
        return true;
    }
};

class InsertRemoveBootEntryCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    QModelIndex index_parent;
    int index;
    BootEntry entry;

public:
    InsertRemoveBootEntryCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent = nullptr)
        : QUndoCommand(description, parent)
        , model{model_}
        , index_parent{index_parent_}
        , index{index_}
        , entry{entry_}
    {
    }

    InsertRemoveBootEntryCommand(const InsertRemoveBootEntryCommand &) = delete;
    InsertRemoveBootEntryCommand &operator=(const InsertRemoveBootEntryCommand &) = delete;

protected:
    void insert()
    {
        model.beginInsertRows(index_parent, index, index);
        model.entries.insert(index, entry);
        model.endInsertRows();
    }

    void remove()
    {
        model.beginRemoveRows(index_parent, index, index);
        model.entries.removeAt(index);
        model.endRemoveRows();
    }
};

class InsertBootEntryCommand: public InsertRemoveBootEntryCommand
{
public:
    InsertBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent = nullptr)
        : InsertRemoveBootEntryCommand(model_, QObject::tr("Insert %1 entry \"%2\" at position %3").arg(model_.name, entry_.getTitle()).arg(index_), index_parent_, index_, entry_, parent)
    {
    }

    InsertBootEntryCommand(const InsertBootEntryCommand &) = delete;
    InsertBootEntryCommand &operator=(const InsertBootEntryCommand &) = delete;

    void undo() override
    {
        remove();
    }

    void redo() override
    {
        insert();
    }
};

class RemoveBootEntryCommand: public InsertRemoveBootEntryCommand
{
public:
    RemoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, QUndoCommand *parent = nullptr)
        : InsertRemoveBootEntryCommand(model_, QObject::tr("Removing %1 entry \"%2\" from position %3").arg(model_.name, model_.entries.at(index_).getTitle()).arg(index_), index_parent_, index_, model_.entries.at(index_), parent)
    {
    }

    RemoveBootEntryCommand(const RemoveBootEntryCommand &) = delete;
    RemoveBootEntryCommand &operator=(const RemoveBootEntryCommand &) = delete;

    void undo() override
    {
        insert();
    }

    void redo() override
    {
        remove();
    }
};

class MoveBootEntryCommand: public QUndoCommand
{
    BootEntryListModel &model;
    const QString title;
    QModelIndex source_parent;
    QModelIndex destination_parent;
    int source_index;
    int destination_index;

public:
    MoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &source_parent_, int source_index_, const QModelIndex &destination_parent_, int destination_index_, QUndoCommand *parent = nullptr)
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

    MoveBootEntryCommand(const MoveBootEntryCommand &) = delete;
    MoveBootEntryCommand &operator=(const MoveBootEntryCommand &) = delete;

    int id() const override
    {
        return 2;
    }

    void undo() override
    {
        model.beginMoveRows(destination_parent, destination_index, destination_index + 1, source_parent, source_index);
        model.entries.move(destination_index, source_index);
        model.endMoveRows();
    }

    void redo() override
    {
        model.beginMoveRows(source_parent, source_index, source_index + 1, destination_parent, destination_index);
        model.entries.move(source_index, destination_index);
        model.endMoveRows();
    }

    bool mergeWith(const QUndoCommand *command) override
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
};

template <class Type>
class SetBootEntryValueCommand: public QUndoCommand
{
    typedef Type BootEntry::*PropertyPtr;

    BootEntryListModel &model;
    const QString title;
    const QModelIndex index;
    const QString name;
    PropertyPtr property;
    Type value;

public:
    SetBootEntryValueCommand(BootEntryListModel &model_, const QModelIndex &index_, const QString &name_, PropertyPtr property_, const Type &value_, QUndoCommand *parent = nullptr)
        : QUndoCommand{"", parent}
        , model{model_}
        , title{model_.entries.at(index_.row()).getTitle()}
        , index{index_}
        , name{name_}
        , property{property_}
        , value{value_}
    {
        setText(QObject::tr("Change %1 entry \"%2\" %3 to \"%4\"").arg(model.name, title, name).arg(value));
    }

    SetBootEntryValueCommand(const SetBootEntryValueCommand &) = delete;
    SetBootEntryValueCommand &operator=(const SetBootEntryValueCommand &) = delete;

    int id() const override
    {
        return 3;
    }

    void undo() override
    {
        redo();
    }

    void redo() override
    {
        auto &entry = model.entries[index.row()];
        auto old_value = entry.*property;
        entry.*property = value;
        value = old_value;
        emit model.dataChanged(index, index, {Qt::EditRole});
    }

    bool mergeWith(const QUndoCommand *command) override
    {
        auto cmd = static_cast<const SetBootEntryValueCommand<Type> *>(command);
        if(&cmd->model != &model)
            return false;

        if(cmd->index != index)
            return false;

        if(cmd->property != property)
            return false;

        auto &entry = model.entries.at(index.row());
        if(value == entry.*property)
            setObsolete(true);

        setText(QObject::tr("Change %1 entry \"%2\" %3 to \"%4\"").arg(model.name, title, name).arg(entry.*property));
        return true;
    }
};

class ChangeOptionalDataFormatCommand: public QUndoCommand
{
    BootEntryListModel &model;
    const QString title;
    const QModelIndex index;
    BootEntry::OptionalDataFormat value;

public:
    ChangeOptionalDataFormatCommand(BootEntryListModel &model_, const QModelIndex &index_, const BootEntry::OptionalDataFormat &value_, QUndoCommand *parent = nullptr)
        : QUndoCommand{"", parent}
        , model{model_}
        , title{model_.entries.at(index_.row()).getTitle()}
        , index{index_}
        , value{value_}
    {
        updateTitle(value);
    }

    ChangeOptionalDataFormatCommand(const ChangeOptionalDataFormatCommand &) = delete;
    ChangeOptionalDataFormatCommand &operator=(const ChangeOptionalDataFormatCommand &) = delete;

    int id() const override
    {
        return 4;
    }

    void undo() override
    {
        redo();
    }

    void redo() override
    {
        auto &entry = model.entries[index.row()];
        auto old_value = entry.optional_data_format;
        entry.changeOptionalDataFormat(value);
        value = old_value;
        emit model.dataChanged(index, index, {Qt::EditRole});
    }

    bool mergeWith(const QUndoCommand *command) override
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

    void updateTitle(BootEntry::OptionalDataFormat val)
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
};

class InsertRemoveBootEntryFilePathCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    const QModelIndex index;
    int row;
    File_path::ANY file_path;

public:
    InsertRemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_, int row_, const File_path::ANY &file_path_, QUndoCommand *parent = nullptr)
        : QUndoCommand(description, parent)
        , model{model_}
        , index{index_}
        , row{row_}
        , file_path{file_path_}
    {
    }

    InsertRemoveBootEntryFilePathCommand(const InsertRemoveBootEntryFilePathCommand &) = delete;
    InsertRemoveBootEntryFilePathCommand &operator=(const InsertRemoveBootEntryFilePathCommand &) = delete;

protected:
    void insert()
    {
        auto &entry = model.entries[index.row()];
        entry.device_path.insert(row, file_path);
        entry.formatDevicePath();
        emit model.dataChanged(index, index, {Qt::EditRole});
    }

    void remove()
    {
        auto &entry = model.entries[index.row()];
        entry.device_path.removeAt(row);
        entry.formatDevicePath();
        emit model.dataChanged(index, index, {Qt::EditRole});
    }
};

class InsertBootEntryFilePathCommand: public InsertRemoveBootEntryFilePathCommand
{
public:
    InsertBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const File_path::ANY &file_path_, QUndoCommand *parent = nullptr)
        : InsertRemoveBootEntryFilePathCommand(model_, QObject::tr("Insert %1 entry \"%2\" file path at position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), index_, row_, file_path_, parent)
    {
    }

    InsertBootEntryFilePathCommand(const InsertBootEntryFilePathCommand &) = delete;
    InsertBootEntryFilePathCommand &operator=(const InsertBootEntryFilePathCommand &) = delete;

    void undo() override
    {
        remove();
    }

    void redo() override
    {
        insert();
    }
};

class RemoveBootEntryFilePathCommand: public InsertRemoveBootEntryFilePathCommand
{
public:
    RemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, QUndoCommand *parent = nullptr)
        : InsertRemoveBootEntryFilePathCommand(model_, QObject::tr("Removing %1 entry \"%2\" file path from position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), index_, row_, model_.entries.at(index_.row()).device_path.at(row_), parent)
    {
    }

    RemoveBootEntryFilePathCommand(const RemoveBootEntryFilePathCommand &) = delete;
    RemoveBootEntryFilePathCommand &operator=(const RemoveBootEntryFilePathCommand &) = delete;

    void undo() override
    {
        insert();
    }

    void redo() override
    {
        remove();
    }
};

class SetBootEntryFilePathCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    QModelIndex index;
    int row;
    File_path::ANY value;

public:
    SetBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const File_path::ANY &value_, QUndoCommand *parent = nullptr)
        : QUndoCommand(QObject::tr("Setting %1 entry \"%2\" file path at position %3").arg(model_.name, model_.entries.at(index_.row()).getTitle()).arg(row_), parent)
        , model{model_}
        , index{index_}
        , row{row_}
        , value{value_}
    {
    }

    SetBootEntryFilePathCommand(const SetBootEntryFilePathCommand &) = delete;
    SetBootEntryFilePathCommand &operator=(const SetBootEntryFilePathCommand &) = delete;

    void undo() override
    {
        redo();
    }

    void redo() override
    {
        auto &entry = model.entries[index.row()];
        auto old_value = entry.device_path[row];
        entry.device_path[row] = value;
        value = old_value;
        entry.formatDevicePath();
        emit model.dataChanged(index, index, {Qt::EditRole});
    }
};

class MoveBootEntryFilePathCommand: public QUndoCommand
{
    BootEntryListModel &model;
    const QString title;
    QModelIndex index;
    int source_row;
    int destination_row;

public:
    MoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int source_row_, int destination_row_, QUndoCommand *parent = nullptr)
        : QUndoCommand("", parent)
        , model{model_}
        , title{model_.entries.at(index_.row()).getTitle()}
        , index{index_}
        , source_row{source_row_}
        , destination_row{destination_row_}
    {
        setText(QObject::tr("Move %1 entry \"%2\" file path from position %3 to %4").arg(model.name, title).arg(source_row).arg(destination_row));
    }

    MoveBootEntryFilePathCommand(const MoveBootEntryFilePathCommand &) = delete;
    MoveBootEntryFilePathCommand &operator=(const MoveBootEntryFilePathCommand &) = delete;

    int id() const override
    {
        return 5;
    }

    void undo() override
    {
        auto &entry = model.entries[index.row()];
        entry.device_path.move(destination_row, source_row);
        entry.formatDevicePath();
        emit model.dataChanged(index, index, {Qt::EditRole});
    }

    void redo() override
    {
        auto &entry = model.entries[index.row()];
        entry.device_path.move(source_row, destination_row);
        entry.formatDevicePath();
        emit model.dataChanged(index, index, {Qt::EditRole});
    }

    bool mergeWith(const QUndoCommand *command) override
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
};
