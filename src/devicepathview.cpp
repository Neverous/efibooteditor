// SPDX-License-Identifier: LGPL-3.0-or-later
#include "devicepathview.h"
#include "filepathdialog.h"

DevicePathView::DevicePathView(QWidget *parent)
    : QListView(parent)
    , delegate{}
    , dialog{std::make_unique<FilePathDialog>(this)}
{
    setItemDelegate(&delegate);
}

void DevicePathView::setReadOnly(bool readonly_)
{
    readonly = readonly_;
}

void DevicePathView::insertRow()
{
    if(readonly)
        return;

    auto index = currentIndex();
    dialog->setReadOnly(readonly);
    dialog->setFilePath(nullptr);
    if(dialog->exec() == QDialog::Accepted)
    {
        auto row = index.row();
        const auto file_path = dialog->toFilePath();
        model()->insertRow(row + 1);
        QVariant _data;
        _data.setValue(&file_path);
        index = model()->index(row + 1, 0);
        model()->setData(index, _data);
        setCurrentIndex(index);
    }
}

void DevicePathView::editCurrentRow()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    dialog->setReadOnly(readonly);
    dialog->setFilePath(model()->data(index).value<const File_path::ANY *>());
    const auto status = dialog->exec();
    if(!readonly && status == QDialog::Accepted)
    {
        const auto file_path = dialog->toFilePath();
        QVariant _data;
        _data.setValue(&file_path);
        model()->setData(index, _data);
        setCurrentIndex(index);
    }
}

void DevicePathView::removeCurrentRow()
{
    if(readonly)
        return;

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

void DevicePathView::moveCurrentRowUp()
{
    if(readonly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() == 0)
        return;

    auto previous_index = index.siblingAtRow(index.row() - 1);
    model()->moveRow(index, index.row(), previous_index, previous_index.row());
    setCurrentIndex(previous_index);
}

void DevicePathView::moveCurrentRowDown()
{
    if(readonly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() >= model()->rowCount() - 1)
        return;

    auto next_index = index.siblingAtRow(index.row() + 1);
    model()->moveRow(index, index.row(), next_index, next_index.row());
    setCurrentIndex(next_index);
}
