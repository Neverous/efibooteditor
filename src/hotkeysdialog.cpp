// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "form/ui_hotkeysdialog.h"
#include "hotkeysdialog.h"

HotKeysDialog::HotKeysDialog(HotKeyListModel &model, QWidget *parent)
    : QDialog{parent}
    , ui{std::make_unique<Ui::HotKeysDialog>()}
{
    ui->setupUi(this);
    ui->hot_keys->setModel(&model);
}

HotKeysDialog::~HotKeysDialog()
{
}

void HotKeysDialog::refreshBootOptions(const BootEntryListModel &model)
{
    ui->hot_keys->refreshBootOptions(model);
}

void HotKeysDialog::setIndexFilter(int index)
{
    ui->index_filter->setText(index != -1 ? toHex(static_cast<uint16_t>(index), 4) : "");
}

void HotKeysDialog::setMaxKeyCount(int keys)
{
    ui->hot_keys->setMaxKeyCount(keys);
}
