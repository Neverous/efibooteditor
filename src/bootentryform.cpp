// SPDX-License-Identifier: LGPL-3.0-or-later
#include "include/bootentryform.h"
#include "ui_bootentryform.h"

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

    ui->description_text->setText(item ? item->description : "");
    paths_proxy_list_model.setBootEntryItem(index, item);
    ui->optional_data_text->setPlainText(item ? item->optional_data : "");
    ui->optional_data_format_combo->setCurrentIndex(item ? item->optional_data_format : 0);
    ui->attribute_active->setChecked(item && (item->attributes & EFIBoot::LOAD_OPTION_ACTIVE));
    ui->attribute_hidden->setChecked(item && (item->attributes & EFIBoot::LOAD_OPTION_HIDDEN));
    ui->attribute_force_reconnect->setChecked(item && (item->attributes & EFIBoot::LOAD_OPTION_FORCE_RECONNECT));
    ui->categoty_combo->setCurrentIndex(item && (item->attributes & EFIBoot::LOAD_OPTION_CATEGORY_APP));

    setDisabled(!item);
}

void BootEntryForm::descriptionEdited(const QString &text)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [&text](BootEntry &entry) {
        entry.description = text;
        return true;
    });
}

void BootEntryForm::optionalDataFormatChanged(int format)
{
    if(!isEnabled())
        return;

    bool success = entries_list_model->changeData(current_index, [&](BootEntry &entry) {
        bool result = entry.change_optional_data_format(static_cast<BootEntry::OptionalDataFormat>(format));
        if(result)
            ui->optional_data_text->setPlainText(entry.optional_data);

        return result;
    });

    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change Optional data format!"));
        ui->optional_data_format_combo->setCurrentIndex(current_item->optional_data_format);
    }
}

void BootEntryForm::optionalDataEdited()
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [&](BootEntry &entry) {
        entry.optional_data = ui->optional_data_text->toPlainText();
        return true;
    });
}

void BootEntryForm::attributeActiveChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry) {
        entry.attributes = (entry.attributes & ~EFIBoot::LOAD_OPTION_ACTIVE) | (state ? EFIBoot::LOAD_OPTION_ACTIVE : EFIBoot::LOAD_OPTION_EMPTY);
        return true;
    });
}

void BootEntryForm::attributeHiddenChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry) {
        entry.attributes = (entry.attributes & ~EFIBoot::LOAD_OPTION_HIDDEN) | (state ? EFIBoot::LOAD_OPTION_HIDDEN : EFIBoot::LOAD_OPTION_EMPTY);
        return true;
    });
}

void BootEntryForm::attributeForceReconnectChanged(int state)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [state](BootEntry &entry) {
        entry.attributes = (entry.attributes & ~EFIBoot::LOAD_OPTION_FORCE_RECONNECT) | (state ? EFIBoot::LOAD_OPTION_FORCE_RECONNECT : EFIBoot::LOAD_OPTION_EMPTY);
        return true;
    });
}

void BootEntryForm::categoryChanged(int index)
{
    if(!isEnabled())
        return;

    entries_list_model->changeData(current_index, [index](BootEntry &entry) {
        entry.attributes = (entry.attributes & ~EFIBoot::LOAD_OPTION_CATEGORY_MASK) | (index ? EFIBoot::LOAD_OPTION_CATEGORY_APP : EFIBoot::LOAD_OPTION_CATEGORY_BOOT);
        return true;
    });
}
