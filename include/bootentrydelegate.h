// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include "bootentrylistmodel.h"
#include "bootentrywidget.h"
#include "qwidgetitemdelegate.h"

class BootEntryDelegate: public QWidgetItemDelegate<BootEntryWidget, const BootEntry *>
{
private:
    BootEntryListModel::Options options{};
    mutable const QModelIndex *currentIndex{nullptr};

public:
    BootEntryDelegate();
    BootEntryDelegate(const BootEntryDelegate &) = delete;
    BootEntryDelegate &operator=(const BootEntryDelegate &) = delete;

    void setOptions(const BootEntryListModel::Options &options_);

protected:
    void setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const override;

private Q_SLOTS:
    void setNextBoot(bool checked) const;
};
