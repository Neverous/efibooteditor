// SPDX-License-Identifier: LGPL-3.0-or-later
#include "include/devicepathlistview.h"
#include "include/devicepathdialog.h"

DevicePathListView::DevicePathListView(QWidget *parent)
    : QListView(parent)
    , dialog{std::make_unique<DevicePathDialog>(this)}
{
    setItemDelegate(&delegate);
}

void DevicePathListView::insertRow()
{
    auto index = currentIndex();
    dialog->setDevicePath(nullptr);
    if(dialog->exec() == QDialog::Accepted)
    {
        auto row = index.row();
        const auto device_path = dialog->toDevicePath();
        model()->insertRow(row + 1);
        QVariant _data;
        _data.setValue(&device_path);
        index = model()->index(row + 1, 0);
        model()->setData(index, _data);
        setCurrentIndex(index);
    }
}

void DevicePathListView::editCurrentRow()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    dialog->setDevicePath(model()->data(index).value<const Device_path::ANY *>());
    if(dialog->exec() == QDialog::Accepted)
    {
        auto row = index.row();
        const auto device_path = dialog->toDevicePath();
        model()->insertRow(row + 1);
        QVariant _data;
        _data.setValue(&device_path);
        model()->setData(index.siblingAtRow(row + 1), _data);
        model()->removeRow(row);
        setCurrentIndex(index);
    }
}

void DevicePathListView::removeCurrentRow()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    auto row = index.row();
    model()->removeRow(row);
    index = index.siblingAtRow(row - 1);
    if(!index.isValid())
        index = model()->index(row, 0);

    setCurrentIndex(index);
}

void DevicePathListView::moveCurrentRowUp()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() == 0)
        return;

    auto previous_index = index.siblingAtRow(index.row() - 1);
    model()->moveRow(index, index.row(), previous_index, previous_index.row());
    setCurrentIndex(previous_index);
}

void DevicePathListView::moveCurrentRowDown()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount() - 1)
        return;

    auto next_index = index.siblingAtRow(index.row() + 1);
    model()->moveRow(index, index.row(), next_index, next_index.row());
    setCurrentIndex(next_index);
}
