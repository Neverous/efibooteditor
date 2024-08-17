// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "bootentrydelegate.h"

#include "bootentry.h"
#include "bootentrylistmodel.h"

BootEntryDelegate::BootEntryDelegate()
    : QWidgetItemDelegate<BootEntryWidget, const BootEntry *>{}
{
    connect(&event_handler, &BootEntryWidget::nextBootClicked, this, &BootEntryDelegate::setNextBoot);
}

void BootEntryDelegate::setOptions(const BootEntryListModel::Options &options_)
{
    options = options_;
}

void BootEntryDelegate::setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const
{
    widget.setReadOnly(options & BootEntryListModel::Option::ReadOnly);
    widget.setIndex(item->index);
    widget.setDescription(item->description);
    widget.setData(!item->is_error ? item->optional_data : item->error);
    widget.showDevicePath(!item->is_error);
    widget.setDevicePath(item->formatDevicePath(false));
    widget.showBootOptions(options & BootEntryListModel::Option::IsBoot);
    widget.setCurrentBoot(item->is_current_boot);
    widget.setNextBoot(item->is_next_boot);

    if(role == Qt::EditRole)
        currentIndex = &index;
}

void BootEntryDelegate::setNextBoot(bool checked) const
{
    if(!currentIndex || !currentIndex->isValid())
        return;

    if(auto item = currentIndex->data().value<const BootEntry *>(); checked != item->is_next_boot)
    {
        auto model = const_cast<BootEntryListModel *>(static_cast<const BootEntryListModel *>(currentIndex->model()));
        model->setNextBootEntry(*currentIndex, checked);
    }
}
