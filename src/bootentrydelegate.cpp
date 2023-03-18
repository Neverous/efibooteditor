// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrydelegate.h"
#include "bootentry.h"
#include "bootentrylistmodel.h"

BootEntryDelegate::BootEntryDelegate()
{
}

void BootEntryDelegate::setReadOnly(bool readonly_)
{
    readonly = readonly_;
}

void BootEntryDelegate::setupWidgetFromItem(Widget &widget, const Item &item) const
{
    widget.setReadOnly(readonly);
    widget.setIndex(item->index);
    widget.setDescription(item->description);
    widget.setData(item->optional_data);
    widget.setDevicePath(item->formatDevicePath(false));
    widget.setCurrentBoot(item->is_current_boot);
    widget.setNextBoot(item->is_next_boot);
}

auto BootEntryDelegate::handleWidgetDelegateEventResult(const QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &, const QModelIndex &index, const Widget &widget, bool result) const -> bool
{
    if(event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease)
        return result;

    auto item = index.data().value<const BootEntry *>();
    if(widget.getNextBoot() != item->is_next_boot)
    {
        auto entries_list_model = static_cast<BootEntryListModel *>(model);
        entries_list_model->setNextBootEntry(index, widget.getNextBoot());
    }

    return result;
}
