// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QAbstractItemModel>
#include <QFlags>
#include <QList>
#include <QUndoStack>

#include "hotkey.h"

class HotKeyListModel: public QAbstractItemModel
{
    Q_OBJECT

    friend class EFIBootData;
    friend class InsertRemoveHotKeyCommand;
    friend class InsertHotKeyCommand;
    friend class RemoveHotKeyCommand;

    template <class Type>
    friend class SetHotKeyValueCommand;
    friend class SetHotKeyKeysCommand;

public:
    enum class Column : uint8_t
    {
        BootOption = 0,
        Keys = 1,
        VendorData = 2,
        Count = 3,
    };

private:
    QVector<QString> header{tr("Boot option"), tr("Hot key"), tr("Vendor data")};
    QVector<HotKey> entries{};
    QUndoStack *undo_stack{nullptr};

public:
    explicit HotKeyListModel(QObject *parent = nullptr);
    HotKeyListModel(const HotKeyListModel &) = delete;
    HotKeyListModel &operator=(const HotKeyListModel &) = delete;

    void setUndoStack(QUndoStack *undo_stack_);
    QUndoStack *getUndoStack() const;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const override { return static_cast<int>(Column::Count); }
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        if(parent.isValid())
            return {};

        if(0 > row || row >= rowCount())
            return {};

        if(0 > column || column >= columnCount())
            return {};

        return createIndex(row, column);
    }

    QModelIndex parent(const QModelIndex & /*child*/) const override { return {}; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void clear();

    const QVector<HotKey> &getEntries() const { return entries; }

private:
    bool appendRow(const HotKey &data, const QModelIndex &parent = QModelIndex());
};
