// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QObject>
#include <QUndoStack>

#include "bootentrylistmodel.h"
#include "hotkeylistmodel.h"

class EFIBootData: public QObject
{
    Q_OBJECT

public:
    BootEntryListModel boot_entries_list_model{tr("Boot"), BootEntryListModel::Option::IsBoot, this};
    BootEntryListModel driver_entries_list_model{tr("Driver"), {}, this};
    BootEntryListModel sysprep_entries_list_model{tr("System Preparation"), {}, this};
    BootEntryListModel platform_recovery_entries_list_model{tr("Platform Recovery"), BootEntryListModel::Option::ReadOnly, this};
    HotKeyListModel hot_keys_list_model{};

    const std::vector<std::tuple<QString, BootEntryListModel &>> BOOT_ENTRIES{
        {"Boot", boot_entries_list_model},
        {"Driver", driver_entries_list_model},
        {"SysPrep", sysprep_entries_list_model},
        {"PlatformRecovery", platform_recovery_entries_list_model},
    };

    QString apple_boot_args{};
    QUndoStack *undo_stack{nullptr};

    uint64_t supported_indications{0};
    uint64_t indications{0};

    uint32_t boot_option_support{0};

    uint16_t timeout{0};
    bool secure_boot{false};
    bool vendor_keys{false};
    bool setup_mode{false};
    bool audit_mode{false};
    bool deployed_mode{false};

public:
    explicit EFIBootData(QObject *parent = nullptr);
    EFIBootData(const EFIBootData &) = delete;
    EFIBootData &operator=(const EFIBootData &) = delete;

    QUndoStack *getUndoStack() const;
    void setUndoStack(QUndoStack *undo_stack_);

public Q_SLOTS:
    void clear();
    void reload();
    void save();
    void import_(const QString &file_name);
    void export_(const QString &file_name);
    void dump(const QString &file_name);

    void setTimeout(uint16_t value);
    void setAppleBootArgs(const QString &text);
    void setOsIndications(uint64_t value);

Q_SIGNALS:
    void error(const QString &message, const QString &details);
    void progress(size_t step, size_t total, const QString &details);
    void done();

    void timeoutChanged(const uint16_t &value);
    void secureBootChanged(bool enabled);
    void vendorKeysChanged(bool enabled);
    void setupModeChanged(bool enabled);
    void auditModeChanged(bool enabled);
    void deployedModeChanged(bool enabled);
    void bootOptionSupportChanged(uint32_t flags);
    void appleBootArgsChanged(const QString &text);
    void osIndicationsSupportedChanged(uint64_t value);
    void osIndicationsChanged(const uint64_t &value);

private:
    void setSecureBoot(bool enabled);
    void setVendorKeys(bool enabled);
    void setSetupMode(bool enabled);
    void setAuditMode(bool enabled);
    void setDeployedMode(bool enabled);
    void setBootOptionSupport(uint32_t flags);
    void setOsIndicationsSupported(uint64_t value);
    void importJSONEFIData(const QJsonObject &input);
    void importRawEFIData(const QJsonObject &input);
};
