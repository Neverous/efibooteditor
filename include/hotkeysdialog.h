// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "bootentrylistmodel.h"
#include "hotkeylistmodel.h"
#include <QDialog>

namespace Ui
{
class HotKeysDialog;
}

class HotKeysDialog: public QDialog
{
    Q_OBJECT

public:
    explicit HotKeysDialog(HotKeyListModel &model, QWidget *parent = nullptr);
    HotKeysDialog(const HotKeysDialog &) = delete;
    HotKeysDialog &operator=(const HotKeysDialog &) = delete;
    ~HotKeysDialog() override;

    void refreshBootOptions(const BootEntryListModel &model);
    void setIndexFilter(int index = -1);
    void setMaxKeyCount(int keys = 3);

private:
    std::unique_ptr<Ui::HotKeysDialog> ui;
};
