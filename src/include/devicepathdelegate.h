// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentry.h"
#include "qwidgetitemdelegate.h"
#include <QLabel>

class DevicePathDelegate: public QWidgetItemDelegate<QLabel, const Device_path::ANY *>
{
protected:
    void setupWidgetFromItem(Widget &widget, const Item &item) const override;
};
