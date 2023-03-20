// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentryform.h"
#include "form/ui_bootentryform.h"

#include <QMessageBox>

BootEntryForm::BootEntryForm(QWidget *parent)
    : QWidget(parent)
    , ui{std::make_unique<Ui::BootEntryForm>()}
{
    ui->setupUi(this);
    ui->device_path->setModel(&device_path_proxy_model);
}

BootEntryForm::~BootEntryForm()
{
}

void BootEntryForm::setReadOnly(bool readonly)
{
    for(auto &widget: findChildren<QLineEdit *>())
        widget->setReadOnly(readonly);

    for(auto &widget: findChildren<QPlainTextEdit *>())
        widget->setReadOnly(readonly);

    for(auto &widget: findChildren<QComboBox *>())
        widget->setDisabled(readonly);

    for(auto &widget: findChildren<QCheckBox *>())
        widget->setDisabled(readonly);

    for(auto &widget: findChildren<QLabel *>())
        widget->setDisabled(readonly);

    ui->device_path->setReadOnly(readonly);
    ui->device_path_actions->setDisabled(readonly);
    ui->optional_data_format_combo->setDisabled(false);
}

void BootEntryForm::setBootEntryListModel(BootEntryListModel &model)
{
    entries_list_model = &model;
    device_path_proxy_model.setBootEntryListModel(model);
}

void BootEntryForm::setItem(const QModelIndex &index, const BootEntry *item)
{
    current_index = index;
    current_item = item;

    setDisabled(true);

    ui->index_text->setText(toHex(item ? item->index : 0u, 4));
    ui->description_text->setText(item ? item->description : "");
    device_path_proxy_model.setBootEntryItem(index, item);
    ui->optional_data_text->setPlainText(item ? item->optional_data : "");
    ui->optional_data_format_combo->setCurrentIndex(item ? static_cast<int>(item->optional_data_format) : 0);
    ui->attribute_active->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::ACTIVE) == EFIBoot::Load_option_attribute::ACTIVE);
    ui->attribute_hidden->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::HIDDEN) == EFIBoot::Load_option_attribute::HIDDEN);
    ui->attribute_force_reconnect->setChecked(item && (item->attributes & EFIBoot::Load_option_attribute::FORCE_RECONNECT) == EFIBoot::Load_option_attribute::FORCE_RECONNECT);
    ui->category_combo->setCurrentIndex(item && (item->attributes & EFIBoot::Load_option_attribute::CATEGORY_APP) == EFIBoot::Load_option_attribute::CATEGORY_APP);

    setDisabled(!item);
}

void BootEntryForm::setIndex(const QString &text)
{
    if(!isEnabled())
        return;

    bool success = false;
    uint16_t index = text.right(text.size() - 2).toUShort(&success, HEX_BASE);
    if(!success)
        return;

    entries_list_model->setEntryIndex(current_index, index);
}

void BootEntryForm::setDescription(const QString &text)
{
    if(!isEnabled())
        return;

    entries_list_model->setEntryDescription(current_index, text);
}

void BootEntryForm::setOptionalDataFormat(int format)
{
    if(!isEnabled() || changing_optional_data_format)
        return;

    changing_optional_data_format = true;
    bool success = entries_list_model->changeEntryOptionalDataFormat(current_index, format);
    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change optional data format!"));
        ui->optional_data_format_combo->setCurrentIndex(static_cast<int>(current_item->optional_data_format));
        changing_optional_data_format = false;
        return;
    }

    ui->optional_data_text->setPlainText(current_item->optional_data);
    changing_optional_data_format = false;
}

void BootEntryForm::optionalDataEdited()
{
    if(!isEnabled() || changing_optional_data_format)
        return;

    entries_list_model->setEntryOptionalData(current_index, ui->optional_data_text->toPlainText());
}

void BootEntryForm::setAttribute(int)
{
    if(!isEnabled())
        return;

    entries_list_model->setEntryAttributes(current_index, getAttributes());
}

uint32_t BootEntryForm::getAttributes() const
{
    uint32_t attr = EFIBoot::Load_option_attribute::EMPTY;
    if(ui->attribute_active->isChecked())
        attr |= EFIBoot::Load_option_attribute::ACTIVE;

    if(ui->attribute_hidden->isChecked())
        attr |= EFIBoot::Load_option_attribute::HIDDEN;

    if(ui->attribute_force_reconnect->isChecked())
        attr |= EFIBoot::Load_option_attribute::FORCE_RECONNECT;

    switch(ui->category_combo->currentIndex())
    {
    case 0:
        attr |= EFIBoot::Load_option_attribute::CATEGORY_BOOT;
        break;

    case 1:
        attr |= EFIBoot::Load_option_attribute::CATEGORY_APP;
        break;
    }

    return attr;
}
