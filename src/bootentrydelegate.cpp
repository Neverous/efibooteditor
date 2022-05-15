// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrydelegate.h"
#include "bootentry.h"
#include "bootentrylistmodel.h"

void BootEntryDelegate::setupWidgetFromItem(Widget &widget, const Item &item) const
{
    widget.set_index(item->index);
    widget.set_description(item->description);
    widget.set_data(item->optional_data);
    widget.set_file_path(item->format_file_path(false));
    widget.set_next_boot(item->is_next_boot);
}

auto BootEntryDelegate::handleWidgetDelegateEventResult(const QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &, const QModelIndex &index, const Widget &widget, bool result) const -> bool
{
    if(event->type() != QEvent::MouseButtonPress && event->type() != QEvent::MouseButtonRelease)
        return result;

    auto item = index.data().value<const BootEntry *>();
    if(widget.get_next_boot() != item->is_next_boot)
    {
        auto entries_list_model = static_cast<BootEntryListModel *>(model);
        entries_list_model->setNextBootEntry(index, widget.get_next_boot());
    }

    return result;
}
