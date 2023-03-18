// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QProgressDialog>
#include <memory>

#include "bootentrylistmodel.h"
#include "bootentrylistview.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class EFIBootEditor;
}
QT_END_NAMESPACE

class EFIBootEditor: public QMainWindow
{
    Q_OBJECT

private:
    enum BootEntryType
    {
        BOOT = 0,
        DRIVER = 1,
        SYSPREP = 2,
        PLATFORM_RECOVERY = 3,
    };

    std::unique_ptr<Ui::EFIBootEditor> ui;
    BootEntryListModel boot_entries_list_model{this};
    BootEntryListModel driver_entries_list_model{this};
    BootEntryListModel sysprep_entries_list_model{this};
    BootEntryListModel platform_recovery_entries_list_model{this, true};
    std::unique_ptr<QMessageBox> confirmation = nullptr;
    std::unique_ptr<QMessageBox> error = nullptr;
    std::unique_ptr<QProgressDialog> progress = nullptr;

    const std::vector<std::tuple<QString, BootEntryListModel &>> BOOT_ENTRIES{
        {"Boot", boot_entries_list_model},
        {"Driver", driver_entries_list_model},
        {"SysPrep", sysprep_entries_list_model},
        {"PlatformRecovery", platform_recovery_entries_list_model},
    };

public:
    explicit EFIBootEditor(QWidget *parent = nullptr);
    EFIBootEditor(const EFIBootEditor &) = delete;
    EFIBootEditor &operator=(const EFIBootEditor &) = delete;
    ~EFIBootEditor() override;

public slots:
    void reload();
    void save();
    void import();
    void export_();
    void dump();
    void reorder();
    void resetBootConfiguration();
    void enableBootEntryEditor(const QModelIndex &index);
    void disableBootEntryEditor();
    void clearBootSettings();
    void switchBootEntryEditor(int index);
    void saveBootConfiguration();
    void importBootConfiguration(const QString &file_name);
    void exportBootConfiguration(const QString &file_name);
    void dumpRawEFIData(const QString &file_name);
    void showAboutBox();
    void reorderBootEntries();

    void removeCurrentBootEntry();
    void moveCurrentBootEntryUp();
    void moveCurrentBootEntryDown();
    void insertBootEntry();

private:
    std::tuple<BootEntryListView &, BootEntryListModel &> getBootEntryList(int index);
    std::tuple<BootEntryListView &, BootEntryListModel &> currentBootEntryList();
    void setupOsIndications(uint64_t value);
    uint64_t getOsIndications() const;
    void setupOsIndicationsSupport(uint64_t value);

    void closeEvent(QCloseEvent *event) override;

    void importJSONEFIData(const QJsonObject &input);
    void importRawEFIData(const QJsonObject &input);

    void showError(const QString &message, const QString &details);
    template <class Receiver, typename Slot>
    void showConfirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot);

    void showProgressBar(size_t step, size_t total, const QString &details);
    void hideProgressBar();
};
