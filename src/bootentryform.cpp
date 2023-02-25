// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentryform.h"
#include "form/ui_bootentryform.h"

#include <QMessageBox>

BootEntryForm::BootEntryForm(QWidget *parent)
    : QWidget(parent)
    , ui{std::make_unique<Ui::BootEntryForm>()}
{
    ui->setupUi(this);
    ui->paths_list->setModel(&paths_proxy_list_model);
}

BootEntryForm::~BootEntryForm()
{
}

void BootEntryForm::setBootEntryListModel(BootEntryListModel &model)
{
    entries_list_model = &model;
    paths_proxy_list_model.setBootEntryListModel(model);
}

void BootEntryForm::setItem(const QModelIndex &index, const BootEntry *item)
{
    current_index = index;
    current_item = item;

    setDisabled(true);

    ui->index_text->setText(toHex(item ? item->index : 0u, 4));
    ui->description_text->setText(item ? item->description : "");
    paths_proxy_list_model.setBootEntryItem(index, item);
    ui->optional_data_text->setPlainText(item ? item->optional_data : "");
    ui->optional_data_format_combo->setCurrentIndex(item ? static_cast<int>(item->optional_data_format) : 0);
    ui->attribute_active->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::ACTIVE) == EFIBoot::Load_option_attribute::ACTIVE);
    ui->attribute_hidden->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::HIDDEN) == EFIBoot::Load_option_attribute::HIDDEN);
    ui->attribute_force_reconnect->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::FORCE_RECONNECT) == EFIBoot::Load_option_attribute::FORCE_RECONNECT);
    ui->categoty_combo->setCurrentIndex(item && (item->attributes & EFIBoot::Load_option_attribute::CATEGORY_APP) == EFIBoot::Load_option_attribute::CATEGORY_APP);

    setDisabled(!item);
}

void BootEntryForm::indexEdited(const QString &text)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [&text](BootEntry &entry)
        {
        bool success = false;
        quint16 index = text.right(text.size()-2).toUShort(&success, HEX_BASE);
        if(success)
            entry.index = index;
        return success; });
}

void BootEntryForm::descriptionEdited(const QString &text)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [&text](BootEntry &entry)
        {
        entry.description = text;
        return true; });
}

void BootEntryForm::optionalDataFormatChanged(int format)
{
    if(!isEnabled())
        return;

    bool success = entries_list_model->changeData(current_index, [&](BootEntry &entry)
        {
        bool result = entry.changeOptionalDataFormat(static_cast<BootEntry::OptionalDataFormat>(format));
        if(result)
            ui->optional_data_text->setPlainText(entry.optional_data);

        return result; });

    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change Optional data format!"));
        ui->optional_data_format_combo->setCurrentIndex(static_cast<int>(current_item->optional_data_format));
    }
}

void BootEntryForm::optionalDataEdited()
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [&](BootEntry &entry)
        {
        entry.optional_data = ui->optional_data_text->toPlainText();
        return true; });
}

void BootEntryForm::attributeActiveChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry)
        {
        entry.attributes = (entry.attributes & ~EFIBoot::Load_option_attribute::ACTIVE) | (state ? EFIBoot::Load_option_attribute::ACTIVE : EFIBoot::Load_option_attribute::EMPTY);
        return true; });
}

void BootEntryForm::attributeHiddenChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry)
        {
        entry.attributes = (entry.attributes & ~EFIBoot::Load_option_attribute::HIDDEN) | (state ? EFIBoot::Load_option_attribute::HIDDEN : EFIBoot::Load_option_attribute::EMPTY);
        return true; });
}

void BootEntryForm::attributeForceReconnectChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry)
        {
        entry.attributes = (entry.attributes & ~EFIBoot::Load_option_attribute::FORCE_RECONNECT) | (state ? EFIBoot::Load_option_attribute::FORCE_RECONNECT : EFIBoot::Load_option_attribute::EMPTY);
        return true; });
}

void BootEntryForm::categoryChanged(int index)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [index](BootEntry &entry)
        {
        entry.attributes = (entry.attributes & ~EFIBoot::Load_option_attribute::CATEGORY_MASK) | (index ? EFIBoot::Load_option_attribute::CATEGORY_APP : EFIBoot::Load_option_attribute::CATEGORY_BOOT);
        return true; });
}
