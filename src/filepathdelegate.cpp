// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "filepathdelegate.h"

void FilePathDelegate::setupWidgetFromItem(Widget &widget, const Item &item) const
{
    widget.setText(std::visit([](const auto &obj)
        { return obj.toString(); },
        *item));
}
