// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "devicepathview.h"

DevicePathView::DevicePathView(QWidget *parent)
    : QListView(parent)
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
        if(!model()->insertRow(row + 1))
            return;

        QVariant _data;
        _data.setValue(&file_path);
        index = model()->index(row + 1, 0);
        if(model()->setData(index, _data))
            setCurrentIndex(index);
    }
}

void DevicePathView::editCurrentRow()
{
    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index))
        return;

    dialog->setReadOnly(readonly);
    dialog->setFilePath(model()->data(index).value<const FilePath::ANY *>());
    const auto status = dialog->exec();
    if(!readonly && status == QDialog::Accepted)
    {
        const auto file_path = dialog->toFilePath();
        QVariant _data;
        _data.setValue(&file_path);
        if(model()->setData(index, _data))
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
}

void DevicePathView::moveCurrentRowUp()
{
    if(readonly)
        return;

    auto index = currentIndex();
    if(!index.isValid() || !model()->checkIndex(index) || index.row() == 0)
        return;

    auto previous_index = index.siblingAtRow(index.row() - 1);
    if(model()->moveRow(index, index.row(), previous_index, previous_index.row()))
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
    if(model()->moveRow(index, index.row(), next_index, next_index.row()))
        setCurrentIndex(next_index);
}
