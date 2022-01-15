// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentrywidget.h"
#include "bootentry.h"
#include "form/ui_bootentrywidget.h"
#include <QMouseEvent>

BootEntryWidget::BootEntryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::BootEntryWidget>())
{
    ui->setupUi(this);
}

BootEntryWidget::~BootEntryWidget()
{
}

void BootEntryWidget::set_description(const QString &description)
{
    ui->description->setText(description);
    ui->description->setStatusTip(description);
    setStatusTip(description);
    setToolTip(description);
}

void BootEntryWidget::set_file_path(const QString &file_path)
{
    ui->file_path->setText(file_path);
    ui->file_path->setStatusTip(file_path);
}

void BootEntryWidget::set_data(const QString &_data)
{
    ui->data->setText(_data);
    ui->data->setStatusTip(_data);
}

auto BootEntryWidget::get_next_boot() const -> bool
{
    return ui->next_boot->isChecked();
}

void BootEntryWidget::set_next_boot(bool next_boot)
{
    ui->next_boot->setChecked(next_boot);
}
