// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QComboBox>

#include "bootentrylistmodel.h"
#include "efikeysequenceedit.h"
#include "qwidgetitemdelegate.h"

class HotKeyBootOptionDelegate: public QWidgetItemDelegate<QComboBox, uint16_t>
{
public:
    HotKeyBootOptionDelegate() = default;
    HotKeyBootOptionDelegate(const HotKeyBootOptionDelegate &) = delete;
    HotKeyBootOptionDelegate &operator=(const HotKeyBootOptionDelegate &) = delete;

    void refreshBootOptions(const BootEntryListModel &model);

protected:
    void setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const override;
    bool setupItemFromWidget(const Widget &widget, Item &item, const QModelIndex &index) const override;

    bool eventFilter(QObject *editor, QEvent *event) override;
};

class HotKeyKeysDelegate: public QWidgetItemDelegate<EFIKeySequenceEdit, const EFIKeySequence *>
{
private:
    qsizetype maximumSequenceLength{3};

public:
    HotKeyKeysDelegate() = default;
    HotKeyKeysDelegate(const HotKeyKeysDelegate &) = delete;
    HotKeyKeysDelegate &operator=(const HotKeyKeysDelegate &) = delete;

    void setMaximumSequenceLength(qsizetype count);

protected:
    void setupWidgetFromItem(Widget &widget, const Item &item, const QModelIndex &index, int role) const override;
    bool setupItemFromWidget(const Widget &widget, Item &item, const QModelIndex &index) const override;
};
