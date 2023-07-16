// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrywidget.h"

#include "compat.h"
#include "form/ui_bootentrywidget.h"
#include <QMouseEvent>

BootEntryWidget::BootEntryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::BootEntryWidget>())
{
    ui->setupUi(this);
    ui->current_boot->setMaximumSize(6, 6);
}

BootEntryWidget::~BootEntryWidget()
{
}

void BootEntryWidget::setReadOnly(bool readonly)
{
    ui->next_boot->setDisabled(readonly);
}

void BootEntryWidget::showBootOptions(bool is_boot)
{
    ui->current_boot->setVisible(is_boot);
    ui->next_boot->setVisible(is_boot);
}

void BootEntryWidget::showDevicePath(bool not_error)
{
    ui->device_path->setVisible(not_error);
}

void BootEntryWidget::setIndex(const uint32_t index)
{
    ui->index->setText(toHex(index, 4));
}

void BootEntryWidget::setDescription(const QString &description)
{
    ui->description->setText(description);
    ui->description->setStatusTip(description);
    setStatusTip(description);
    setToolTip(description);
}

void BootEntryWidget::setDevicePath(const QString &device_path)
{
    ui->device_path->setText(device_path);
    ui->device_path->setStatusTip(device_path);
}

void BootEntryWidget::setData(const QString &_data)
{
    ui->data->setText(_data);
    ui->data->setStatusTip(_data);
    ui->data->setHidden(_data.isEmpty());
}

auto BootEntryWidget::getNextBoot() const -> bool
{
    return ui->next_boot->isChecked();
}

void BootEntryWidget::setNextBoot(bool next_boot)
{
    ui->next_boot->setChecked(next_boot);
}

auto BootEntryWidget::getCurrentBoot() const -> bool
{
    return ui->current_boot->isChecked();
}

void BootEntryWidget::setCurrentBoot(bool current_boot)
{
    ui->current_boot->setChecked(current_boot);
    ui->current_boot->setHidden(!current_boot);
}
