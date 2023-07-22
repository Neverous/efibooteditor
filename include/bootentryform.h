// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QWidget>
#include <memory>

#include "bootentry.h"
#include "bootentrylistmodel.h"
#include "devicepathproxymodel.h"

namespace Ui
{
class BootEntryForm;
}

class BootEntryForm: public QWidget
{
    Q_OBJECT

private:
    std::unique_ptr<Ui::BootEntryForm> ui;
    BootEntryListModel *entries_list_model{nullptr};
    DevicePathProxyModel device_path_proxy_model{};
    QModelIndex current_index{};
    const BootEntry *current_item{nullptr};
    bool changing_optional_data_format{false};

public:
    explicit BootEntryForm(QWidget *parent = nullptr);
    BootEntryForm(const BootEntryForm &) = delete;
    BootEntryForm &operator=(const BootEntryForm &) = delete;
    ~BootEntryForm() override;

    void setReadOnly(bool readonly);
    void showCategory(bool visible);

    void setBootEntryListModel(BootEntryListModel &model);
    void setItem(const QModelIndex &index, const BootEntry *item);

private slots:
    void setIndex(const QString &text);
    void setDescription(const QString &text);
    void setOptionalDataFormat(int format);
    void optionalDataEdited();
    void setAttribute(int state);

private:
    uint32_t getAttributes() const;
};
