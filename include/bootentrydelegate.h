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

public:
    BootEntryDelegate() = default;
    BootEntryDelegate(const BootEntryDelegate &) = delete;
    BootEntryDelegate &operator=(const BootEntryDelegate &) = delete;

    void setOptions(const BootEntryListModel::Options &options_);

protected:
    void setupWidgetFromItem(Widget &widget, const Item &item) const override;
    bool handleWidgetDelegateEventResult(const QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index, const Widget &widget, bool result) const override;
};
