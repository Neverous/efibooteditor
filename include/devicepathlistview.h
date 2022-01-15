// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "devicepathdelegate.h"
#include "devicepathdialog.h"
#include <QListView>

class DevicePathListView: public QListView
{
    Q_OBJECT

private:
    DevicePathDelegate delegate;
    std::unique_ptr<DevicePathDialog> dialog = nullptr;

public:
    explicit DevicePathListView(QWidget *parent = nullptr);
    DevicePathListView(const DevicePathListView &) = delete;
    DevicePathListView &operator=(const DevicePathListView &) = delete;

public slots:
    void insertRow();
    void editCurrentRow();
    void removeCurrentRow();
    void moveCurrentRowUp();
    void moveCurrentRowDown();
};
