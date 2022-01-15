// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include <QAbstractListModel>
#include <QList>

typedef std::function<bool(BootEntry &)> Change_fn;

class BootEntryListModel: public QAbstractListModel
{
    Q_OBJECT

private:
    QVector<BootEntry> entries = {};
    QModelIndex next_boot = {};

public:
    explicit BootEntryListModel(QObject *parent = nullptr);
    BootEntryListModel(const BootEntryListModel &) = delete;
    BootEntryListModel &operator=(const BootEntryListModel &) = delete;

    // Header:
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override { return {}; }

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool changeData(const QModelIndex &index, const Change_fn &change_fn, int role = Qt::EditRole);
    void setNextBootEntry(const QModelIndex &index, bool value);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool appendRow(const BootEntry &data, const QModelIndex &parent = QModelIndex());

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    // Move data
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    void clear();

    const QVector<BootEntry> &getEntries() const { return entries; }
};
