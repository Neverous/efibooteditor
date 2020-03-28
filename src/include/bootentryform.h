// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QWidget>
#include <memory>

#include "bootentry.h"
#include "bootentrylistmodel.h"
#include "devicepathproxylistmodel.h"

namespace Ui
{
class BootEntryForm;
}

class BootEntryForm: public QWidget
{
    Q_OBJECT

public:
    explicit BootEntryForm(QWidget *parent = nullptr);
    BootEntryForm(const BootEntryForm &) = delete;
    BootEntryForm &operator=(const BootEntryForm &) = delete;
    ~BootEntryForm();

    void setBootEntryListModel(BootEntryListModel &model);
    void setItem(const QModelIndex &index, const BootEntry *item);

private:
    std::unique_ptr<Ui::BootEntryForm> ui;
    BootEntryListModel *entries_list_model = nullptr;
    DevicePathProxyListModel paths_proxy_list_model{};
    QModelIndex current_index = {};
    const BootEntry *current_item = nullptr;

private slots:
    void descriptionEdited(const QString &text);
    void optionalDataFormatChanged(int format);
    void optionalDataEdited();
    void attributeActiveChanged(int state);
    void attributeHiddenChanged(int state);
    void attributeForceReconnectChanged(int state);
    void categoryChanged(int index);
};
