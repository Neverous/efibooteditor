// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "filepathdelegate.h"
#include "filepathdialog.h"
#include <QListView>

class DevicePathView: public QListView
{
    Q_OBJECT

private:
    FilePathDelegate delegate{};
    std::unique_ptr<FilePathDialog> dialog{nullptr};
    bool readonly{false};

public:
    explicit DevicePathView(QWidget *parent = nullptr);
    DevicePathView(const DevicePathView &) = delete;
    DevicePathView &operator=(const DevicePathView &) = delete;

    void setReadOnly(bool readonly);

public Q_SLOTS:
    void insertRow();
    void editCurrentRow();
    void removeCurrentRow() const;
    void moveCurrentRowUp();
    void moveCurrentRowDown();
};
