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

    void set_description(const QString &description);
    void set_file_path(const QString &file_path);
    void set_data(const QString &data);
    bool get_next_boot() const;
    void set_next_boot(bool next_boot);
};
