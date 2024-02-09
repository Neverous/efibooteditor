// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QListView>

#include "bootentrydelegate.h"
#include "bootentrylistmodel.h"

class BootEntryListView: public QListView
{
    Q_OBJECT

private:
    BootEntryDelegate delegate{};
    BootEntryListModel::Options options{};

public:
    explicit BootEntryListView(QWidget *parent = nullptr);
    BootEntryListView(const BootEntryListView &) = delete;
    BootEntryListView &operator=(const BootEntryListView &) = delete;

    void setModel(QAbstractItemModel *model) override { QListView::setModel(model); }
    void setModel(BootEntryListModel *model);

protected:
    void selectionChanged(const QItemSelection &selection, const QItemSelection &) override;

signals:
    void selected(const QModelIndex &index);

public slots:
    void insertRow();
    void duplicateRow();
    void removeCurrentRow();
    void moveCurrentRowUp();
    void moveCurrentRowDown();

    void rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow);
    void rowsChanged(const QModelIndex &parent, int first, int last);
};
