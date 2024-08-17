// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QWidget>
#include <memory>

class BootEntry;
class QButtonGroup;

namespace Ui
{
class BootEntryWidget;
}

class BootEntryWidget: public QWidget
{
    Q_OBJECT

private:
    std::unique_ptr<Ui::BootEntryWidget> ui;

public:
    explicit BootEntryWidget(QWidget *parent = nullptr);
    BootEntryWidget(const BootEntryWidget &) = delete;
    BootEntryWidget &operator=(const BootEntryWidget &) = delete;
    ~BootEntryWidget() override;

    void setReadOnly(bool readonly);
    void showBootOptions(bool is_boot);
    void showDevicePath(bool not_error);

    void setIndex(const uint32_t index);
    void setDescription(const QString &description);
    void setDevicePath(const QString &device_path);
    void setData(const QString &data);
    bool getNextBoot() const;
    void setNextBoot(bool next_boot);
    bool getCurrentBoot() const;
    void setCurrentBoot(bool current_boot);

Q_SIGNALS:
    void nextBootClicked(bool checked);
};
