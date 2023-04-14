// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QAbstractListModel>
#include <QFlags>
#include <QList>
#include <QUndoStack>

typedef std::function<bool(BootEntry &)> Change_fn;

class BootEntryListModel: public QAbstractListModel
{
    Q_OBJECT

    friend class EFIBootData;
    friend class InsertRemoveBootEntryCommand;
    friend class InsertBootEntryCommand;
    friend class RemoveBootEntryCommand;
    friend class MoveBootEntryCommand;

    template <class Type>
    friend class SetBootEntryValueCommand;
    friend class ChangeOptionalDataFormatCommand;
    friend class SetBootEntryFilePathCommand;
    friend class InsertRemoveBootEntryFilePathCommand;
    friend class InsertBootEntryFilePathCommand;
    friend class RemoveBootEntryFilePathCommand;
    friend class MoveBootEntryFilePathCommand;

public:
    enum Option
    {
        ReadOnly = 0x1,
        IsBoot = 0x2,
    };
    Q_DECLARE_FLAGS(Options, Option)

    const QString name;
    const Options options;

private:
    QVector<BootEntry> entries{};
    QModelIndex next_boot{};
    QUndoStack *undo_stack{nullptr};

public:
    explicit BootEntryListModel(const QString &name_, const Options &options_ = {}, QObject *parent = nullptr);
    BootEntryListModel(const BootEntryListModel &) = delete;
    BootEntryListModel &operator=(const BootEntryListModel &) = delete;

    void setUndoStack(QUndoStack *undo_stack_);
    QUndoStack *getUndoStack() const;

    // Header:
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override { return {}; }

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setNextBootEntry(const QModelIndex &index, bool value);

    void setEntryFilePath(const QModelIndex &index, int row, const File_path::ANY &file_path);
    void insertEntryFilePath(const QModelIndex &index, int row, const File_path::ANY &file_path);
    void removeEntryFilePath(const QModelIndex &index, int row);
    void moveEntryFilePath(const QModelIndex &index, int source_row, int destination_row);
    void clearEntryDevicePath(const QModelIndex &index);
    void setEntryIndex(const QModelIndex &index, uint16_t value);
    void setEntryDescription(const QModelIndex &index, const QString &text);
    bool changeEntryOptionalDataFormat(const QModelIndex &index, int format);
    void setEntryOptionalData(const QModelIndex &index, const QString &text);
    void setEntryAttributes(const QModelIndex &index, uint32_t value);
    void setEntryNextBoot(const QModelIndex &index, bool value);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Move data
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    void clear();

    const QVector<BootEntry> &getEntries() const { return entries; }

private:
    bool appendRow(const BootEntry &data, const QModelIndex &parent = QModelIndex());
};
