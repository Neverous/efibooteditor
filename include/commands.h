// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "efibootdata.h"
#include <QUndoCommand>

template <class Type, class SignalPtr>
class SetEFIBootDataValueCommand: public QUndoCommand
{
    using PropertyPtr = Type EFIBootData::*;

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
        const auto cmd = static_cast<add_const_t<decltype(this)>>(command);
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

template <class Type, class SignalPtr>
SetEFIBootDataValueCommand(EFIBootData &data_, const QString &name_, typename SetEFIBootDataValueCommand<Type, SignalPtr>::PropertyPtr property_, SignalPtr signal_, const Type &value_, QUndoCommand *parent) -> SetEFIBootDataValueCommand<Type, SignalPtr>;

class InsertRemoveBootEntryCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    QModelIndex index_parent;
    BootEntry entry;
    int index;

public:
    InsertRemoveBootEntryCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent = nullptr);
    InsertRemoveBootEntryCommand(const InsertRemoveBootEntryCommand &) = delete;
    InsertRemoveBootEntryCommand &operator=(const InsertRemoveBootEntryCommand &) = delete;

protected:
    int id() const override;
    void insert();
    void remove();
};

class InsertBootEntryCommand: public InsertRemoveBootEntryCommand
{
public:
    InsertBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, const BootEntry &entry_, QUndoCommand *parent = nullptr);
    InsertBootEntryCommand(const InsertBootEntryCommand &) = delete;
    InsertBootEntryCommand &operator=(const InsertBootEntryCommand &) = delete;

    void undo() override;
    void redo() override;
};

class RemoveBootEntryCommand: public InsertRemoveBootEntryCommand
{
public:
    RemoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &index_parent_, int index_, QUndoCommand *parent = nullptr);
    RemoveBootEntryCommand(const RemoveBootEntryCommand &) = delete;
    RemoveBootEntryCommand &operator=(const RemoveBootEntryCommand &) = delete;

    void undo() override;
    void redo() override;
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
    MoveBootEntryCommand(BootEntryListModel &model_, const QModelIndex &source_parent_, int source_index_, const QModelIndex &destination_parent_, int destination_index_, QUndoCommand *parent = nullptr);
    MoveBootEntryCommand(const MoveBootEntryCommand &) = delete;
    MoveBootEntryCommand &operator=(const MoveBootEntryCommand &) = delete;

    int id() const override;
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
};

template <class Type>
class SetBootEntryValueCommand: public QUndoCommand
{
    using PropertyPtr = Type BootEntry::*;

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
        setText(QObject::tr("Change %1 entry \"%2\" %3 to \"%4\"").arg(model.name, title, name).arg(static_cast<underlying_type_t<Type>>(value)));
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

        setText(QObject::tr("Change %1 entry \"%2\" %3 to \"%4\"").arg(model.name, title, name).arg(static_cast<underlying_type_t<Type>>(entry.*property)));
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
    ChangeOptionalDataFormatCommand(BootEntryListModel &model_, const QModelIndex &index_, const BootEntry::OptionalDataFormat &value_, QUndoCommand *parent = nullptr);
    ChangeOptionalDataFormatCommand(const ChangeOptionalDataFormatCommand &) = delete;
    ChangeOptionalDataFormatCommand &operator=(const ChangeOptionalDataFormatCommand &) = delete;

    int id() const override;
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
    void updateTitle(BootEntry::OptionalDataFormat val);
};

class InsertRemoveBootEntryFilePathCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    FilePath::ANY file_path;
    const QModelIndex index;
    int row;

public:
    InsertRemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QString &description, const QModelIndex &index_, int row_, const FilePath::ANY &file_path_, QUndoCommand *parent = nullptr);
    InsertRemoveBootEntryFilePathCommand(const InsertRemoveBootEntryFilePathCommand &) = delete;
    InsertRemoveBootEntryFilePathCommand &operator=(const InsertRemoveBootEntryFilePathCommand &) = delete;

protected:
    int id() const override;
    void insert();
    void remove();
};

class InsertBootEntryFilePathCommand: public InsertRemoveBootEntryFilePathCommand
{
public:
    InsertBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const FilePath::ANY &file_path_, QUndoCommand *parent = nullptr);
    InsertBootEntryFilePathCommand(const InsertBootEntryFilePathCommand &) = delete;
    InsertBootEntryFilePathCommand &operator=(const InsertBootEntryFilePathCommand &) = delete;

    void undo() override;
    void redo() override;
};

class RemoveBootEntryFilePathCommand: public InsertRemoveBootEntryFilePathCommand
{
public:
    RemoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, QUndoCommand *parent = nullptr);
    RemoveBootEntryFilePathCommand(const RemoveBootEntryFilePathCommand &) = delete;
    RemoveBootEntryFilePathCommand &operator=(const RemoveBootEntryFilePathCommand &) = delete;

    void undo() override;
    void redo() override;
};

class SetBootEntryFilePathCommand: public QUndoCommand
{
protected:
    BootEntryListModel &model;
    QModelIndex index;
    FilePath::ANY value;
    int row;

public:
    SetBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int row_, const FilePath::ANY &value_, QUndoCommand *parent = nullptr);
    SetBootEntryFilePathCommand(const SetBootEntryFilePathCommand &) = delete;
    SetBootEntryFilePathCommand &operator=(const SetBootEntryFilePathCommand &) = delete;

    int id() const override;
    void undo() override;
    void redo() override;
};

class MoveBootEntryFilePathCommand: public QUndoCommand
{
    BootEntryListModel &model;
    const QString title;
    QModelIndex index;
    int source_row;
    int destination_row;

public:
    MoveBootEntryFilePathCommand(BootEntryListModel &model_, const QModelIndex &index_, int source_row_, int destination_row_, QUndoCommand *parent = nullptr);
    MoveBootEntryFilePathCommand(const MoveBootEntryFilePathCommand &) = delete;
    MoveBootEntryFilePathCommand &operator=(const MoveBootEntryFilePathCommand &) = delete;

    int id() const override;
    void undo() override;
    void redo() override;
    bool mergeWith(const QUndoCommand *command) override;
};
