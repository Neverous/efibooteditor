// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include "qwidgetitemdelegate.h"
#include <QLabel>

class FilePathDelegate: public QWidgetItemDelegate<QLabel, const FilePath::ANY *>
{
public:
    FilePathDelegate() = default;
    FilePathDelegate(const FilePathDelegate &) = delete;
    FilePathDelegate &operator=(const FilePathDelegate &) = delete;

protected:
    void setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const override;
};
