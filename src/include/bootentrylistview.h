// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QListView>

#include "bootentrydelegate.h"

class BootEntryListView: public QListView
{
    Q_OBJECT

private:
    BootEntryDelegate delegate = {};

public:
    explicit BootEntryListView(QWidget *parent = nullptr);

protected:
    void selectionChanged(const QItemSelection &selection, const QItemSelection &) override;

signals:
    void selected(const QModelIndex &index);

public slots:
    void insertRow();
    void removeCurrentRow();
    void moveCurrentRowUp();
    void moveCurrentRowDown();
};
