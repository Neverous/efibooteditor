// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include "bootentrylistmodel.h"
#include <QAbstractListModel>

class DevicePathProxyModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit DevicePathProxyModel(QObject *parent = nullptr);
    DevicePathProxyModel(const DevicePathProxyModel &) = delete;
    DevicePathProxyModel &operator=(const DevicePathProxyModel &) = delete;

    void setBootEntryListModel(BootEntryListModel &model);
    void setBootEntryItem(const QModelIndex &index, const BootEntry *item);

    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const override { return {}; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    void clear();

private:
    BootEntryListModel *boot_entry_list_model = nullptr;
    QModelIndex boot_entry_index = {};
    const QVector<FilePath::ANY> *boot_entry_device_path = nullptr;
};
