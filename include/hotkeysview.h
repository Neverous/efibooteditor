// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentrylistmodel.h"
#include "hotkeydelegate.h"
#include <QTreeView>

class HotKeysView: public QTreeView
{
    Q_OBJECT

private:
    HotKeyBootOptionDelegate bootOptionDelegate{};
    HotKeyKeysDelegate keysDelegate{};

public:
    explicit HotKeysView(QWidget *parent = nullptr);
    HotKeysView(const HotKeysView &) = delete;
    HotKeysView &operator=(const HotKeysView &) = delete;

    void refreshBootOptions(const BootEntryListModel &model);
    void setMaxKeyCount(qsizetype keys);

public Q_SLOTS:
    void insertRow();
    void removeCurrentRow() const;
    void setFilter(const QString &filter);
};
