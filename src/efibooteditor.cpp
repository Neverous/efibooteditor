// SPDX-License-Identifier: LGPL-3.0-or-later
#include "efibooteditor.h"

#include "bootentry.h"
#include "efiboot.h"
#include "form/ui_efibooteditor.h"
#include <QButtonGroup>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMetaMethod>
#include <cctype>
#include <unordered_set>

static bool is_bootentry(const std::tstring &name, const std::tstring &prefix)
{
    if(name.length() != prefix.length() + 4 || name.substr(0, prefix.length()) != prefix)
        return false;

    auto suffix = name.substr(prefix.length());
    if(!isxnumber(suffix))
        return false;

    return true;
}

EFIBootEditor::EFIBootEditor(QWidget *parent)
    : QMainWindow{parent}
    , ui{std::make_unique<Ui::EFIBootEditor>()}
    , confirmation{std::make_unique<QMessageBox>(QMessageBox::Question, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , error{std::make_unique<QMessageBox>(QMessageBox::Critical, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , progress{std::make_unique<QProgressDialog>(tr("Working..."), nullptr, 0, 0, this)}
{
    ui->setupUi(this);
    ui->boot_entries_list->setModel(&boot_entries_list_model);
    ui->driver_entries_list->setModel(&driver_entries_list_model);
    ui->sysprep_entries_list->setModel(&sysprep_entries_list_model);
    ui->platform_recovery_entries_list->setModel(&platform_recovery_entries_list_model);
    ui->entry_form->setBootEntryListModel(boot_entries_list_model);
    progress->setWindowModality(Qt::WindowModal);
    ui->entries->setCurrentIndex(0);
    clearBootSettings();
}

EFIBootEditor::~EFIBootEditor()
{
}

void EFIBootEditor::reload()
{
    showConfirmation(
        tr("Are you sure you want to reload the entries?<br/>ALL of your changes will be lost!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::resetBootConfiguration);
}

void EFIBootEditor::resetBootConfiguration()
{
    disableBootEntryEditor();
    ui->entries->setCurrentIndex(0);
    clearBootSettings();

    showProgressBar(0, 1, tr("Loading EFI Boot Manager entries..."));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;

    // clang-format off
    const auto name_to_guid = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view) { return guid == EFIBoot::efi_guid_global; },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching EFI Boot Manager entries...")); });
    // clang-format on

    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;

    auto process_entry = [&](const auto &name, const auto &read_fn, const auto &process_fn, bool optional = false)
    {
        const auto tname = QStringToStdTString(name);
        if(!name_to_guid.count(tname))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(name));

            return;
        }

        showProgressBar(step++, total_steps, tr("Processing EFI Boot Manager entries (%1)...").arg(name));
        const auto variable = read_fn(name_to_guid.at(tname), tname);
        if(!variable)
        {
            errors.push_back(tr("%1: failed deserialization").arg(name));
            return;
        }

        const auto &[value, attributes] = *variable;
        process_fn(value, attributes);
    };

    // clang-format off
    process_entry("Timeout", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        ui->timeout_number->setValue(value);
    }, true);

    process_entry("BootCurrent", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        current_boot = value;
    }, true);

    process_entry("BootNext", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        next_boot = value;
    }, true);

    process_entry("SecureBoot", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->secure_boot->setChecked(value);
    }, true);

    process_entry("VendorKeys", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->vendor_keys->setChecked(value);
    }, true);

    process_entry("SetupMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->setup_mode->setChecked(value);
    }, true);

    process_entry("AuditMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->audit_mode->setChecked(value);
    }, true);

    process_entry("DeployedMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->deployed_mode->setChecked(value);
    }, true);

    process_entry("OsIndicationsSupported", EFIBoot::get_variable<uint64_t>, [&](const uint64_t &value, const auto &)
    {
        setupOsIndicationsSupport(value);
    }, true);

    process_entry("OsIndications", EFIBoot::get_variable<uint64_t>, [&](const uint64_t &value, const auto &)
    {
        setupOsIndications(value);
    }, true);
    // clang-format on

    for(auto [prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        std::unordered_set<unsigned long> ordered_entry;

        // clang-format off
        process_entry(order_name, EFIBoot::get_list_variable<uint16_t>, [&](const std::vector<uint16_t> &value, const auto &)
        {
            order = value;
            for(const auto &index: order)
                ordered_entry.insert(index);
        }, true);
        // clang-format on

        // Add entries not in BootOrder at the end
        for(const auto &[tname, guid]: name_to_guid)
        {
            (void)guid;
            if(!is_bootentry(tname, QStringToStdTString(prefix)))
                continue;

            const uint16_t index = static_cast<uint16_t>(std::stoul(tname.substr(static_cast<size_t>(prefix.size())), nullptr, HEX_BASE));
            if(ordered_entry.count(index))
                continue;

            order.push_back(index);
            ordered_entry.insert(index);
        }

        model.clear();
        for(const auto &index: order)
        {
            const auto qname = toHex(index, 4, prefix);

            // clang-format off
            process_entry(qname, EFIBoot::get_variable<EFIBoot::Load_option>, [&](const EFIBoot::Load_option &value, const uint32_t &attributes)
            {
                // Translate STL to QTL
                auto entry = BootEntry::fromEFIBootLoadOption(value);
                entry.index = index;
                entry.efi_attributes = attributes;
                if(prefix == "Boot")
                {
                    entry.is_current_boot = current_boot == static_cast<int>(index);
                    entry.is_next_boot = next_boot == static_cast<int>(index);
                }
                model.appendRow(entry);
            });
            // clang-format on
        }
    }

    // Apple
    const auto boot_args = EFIBoot::get_variable<std::string>(EFIBoot::efi_guid_apple, _T("boot-args"));
    if(boot_args)
    {
        const auto &[value, attributes] = *boot_args;
        (void)attributes;
        ui->boot_args_text->setText(QString::fromStdString(value));
    }

    if(!errors.isEmpty())
        showError(tr("Error loading entries"), tr("Failed to load some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::reorder()
{
    showConfirmation(
        tr("Are you sure you want to reorder the boot entries?<br/>All indexes will be overwritten!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::reorderBootEntries);
}

void EFIBootEditor::enableBootEntryEditor(const QModelIndex &index)
{
    if(!index.isValid())
        return disableBootEntryEditor();

    const auto item = index.data().value<const BootEntry *>();
    ui->entry_form->setItem(index, item);
}

void EFIBootEditor::disableBootEntryEditor()
{
    ui->entry_form->setItem({}, nullptr);
}

void EFIBootEditor::clearBootSettings()
{
    ui->timeout_number->setValue(0);
    ui->secure_boot->setChecked(false);
    ui->vendor_keys->setChecked(false);
    ui->setup_mode->setChecked(false);
    ui->audit_mode->setChecked(false);
    ui->deployed_mode->setChecked(false);
    setupOsIndicationsSupport(0);
    setupOsIndications(0);
    ui->boot_args_text->setText("");
    ui->settings->setCurrentIndex(0);
}

void EFIBootEditor::switchBootEntryEditor(int index)
{
    auto [list, model] = getBootEntryList(index);
    ui->entry_form->setBootEntryListModel(model);
    list.setCurrentIndex(list.currentIndex());
    enableBootEntryEditor(list.currentIndex());
    ui->entry_form->setReadOnly(model.readonly);
    ui->entries_actions->setDisabled(model.readonly);
}

void EFIBootEditor::save()
{
    showConfirmation(
        tr("Are you sure you want to save?<br/>Your EFI configuration will be overwritten!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::saveBootConfiguration);
}

void EFIBootEditor::saveBootConfiguration()
{
    showProgressBar(0, 1, tr("Saving EFI Boot Manager entries..."));
    int32_t next_boot = -1;

    // clang-format off
    auto old_entries = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view) { return guid == EFIBoot::efi_guid_global; },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching old EFI Boot Manager entries...")); });
    // clang-format on

    size_t step = 0;
    size_t total_steps = 6u
        + static_cast<size_t>(boot_entries_list_model.getEntries().size())
        + static_cast<size_t>(driver_entries_list_model.getEntries().size())
        + static_cast<size_t>(sysprep_entries_list_model.getEntries().size())
        + static_cast<size_t>(platform_recovery_entries_list_model.getEntries().size())
        + old_entries.size();

    // Save entries
    for(auto [prefix, model]: BOOT_ENTRIES)
    {
        if(model.readonly)
            continue;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        QSet<uint16_t> saved;
        for(const auto &entry: model.getEntries())
        {
            const auto qname = toHex(entry.index, 4, prefix);
            if(saved.contains(entry.index))
                return showError(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(qname, entry.description));

            saved.insert(entry.index);
            showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg(qname));
            if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
            {
                order.push_back(entry.index);
                if(entry.is_next_boot)
                    next_boot = entry.index;
            }

            const std::tstring tname = QStringToStdTString(qname);
            if(auto _entry = old_entries.find(tname); _entry != old_entries.end())
                old_entries.erase(_entry);

            const auto load_option = entry.toEFIBootLoadOption();
            if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, tname, EFIBoot::Variable<EFIBoot::Load_option>{load_option, entry.efi_attributes}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
                return showError(tr("Error saving %1").arg(qname), QStringFromStdTString(EFIBoot::get_error_trace()));
        }

        for(const auto &[tname, guid]: old_entries)
        {
            if(!is_bootentry(tname, QStringToStdTString(prefix)))
                continue;

            showProgressBar(step++, total_steps, tr("Removing old EFI Boot Manager entries (%1)...").arg(QStringFromStdTString(tname)));
            if(!EFIBoot::del_variable(guid, tname))
                return showError(tr("Error removing %1").arg(QStringFromStdTString(tname)), QStringFromStdTString(EFIBoot::get_error_trace()));
        }

        // Save order
        if(order.empty())
        {
            showProgressBar(step++, total_steps, tr("Removing EFI Boot Manager entries (%1)...").arg(order_name));
            if(EFIBoot::get_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name)) && !EFIBoot::del_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name)))
                return showError(tr("Error removing %1").arg(order_name), QStringFromStdTString(EFIBoot::get_error_trace()));
        }
        else
        {
            showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg(order_name));
            if(!EFIBoot::set_list_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name), EFIBoot::Variable<std::vector<uint16_t>>{order, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
                return showError(tr("Error saving %1").arg(order_name), QStringFromStdTString(EFIBoot::get_error_trace()));
        }
    }

    // Save next boot
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("BootNext"));
    if(next_boot != -1 && !EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("BootNext"), EFIBoot::Variable<uint16_t>{static_cast<uint16_t>(next_boot), EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("BootNext"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Save timeout
    const auto timeout = static_cast<uint16_t>(ui->timeout_number->value());
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("Timeout"));
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("Timeout"), EFIBoot::Variable<uint16_t>{timeout, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("Timeout"), QStringFromStdTString(EFIBoot::get_error_trace()));

    const uint64_t indications = getOsIndications();
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("OsIndications"));
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("OsIndications"), EFIBoot::Variable<uint64_t>{indications, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("OsIndications"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Apple
    auto boot_args = ui->boot_args_text->text();
    if(boot_args.isEmpty())
    {
        showProgressBar(step++, total_steps, tr("Removing EFI Boot Manager entries (%1)...").arg("Apple/boot-args"));
        if(EFIBoot::get_variable(EFIBoot::efi_guid_apple, _T("boot-args")) && !EFIBoot::del_variable(EFIBoot::efi_guid_apple, _T("boot-args")))
            return showError(tr("Error removing %1").arg("Apple/boot-args"), QStringFromStdTString(EFIBoot::get_error_trace()));
    }
    else
    {
        showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("Apple/boot-args"));
        if(!EFIBoot::set_variable(EFIBoot::efi_guid_apple, _T("boot-args"), EFIBoot::Variable<std::string>{boot_args.toStdString(), EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
            return showError(tr("Error saving %1").arg("Apple/boot-args"), QStringFromStdTString(EFIBoot::get_error_trace()));
    }

    hideProgressBar();
}

void EFIBootEditor::import()
{
    const QString file_name = QFileDialog::getOpenFileName(this, tr("Open boot configuration dump"), "", tr("JSON documents (*.json)"));
    if(!file_name.isEmpty())
        importBootConfiguration(file_name);

    ui->entries->setCurrentIndex(0);
    ui->settings->setCurrentIndex(0);
}

void EFIBootEditor::importBootConfiguration(const QString &file_name)
{
    disableBootEntryEditor();
    ui->entries->setCurrentIndex(0);
    clearBootSettings();

    showProgressBar(0, 1, tr("Importing boot configuration..."));
    QFile import_file(file_name);
    if(!import_file.open(QIODevice::ReadOnly))
        return showError(tr("Error importing boot configuration"), tr("Couldn't open selected file (%1).").arg(file_name));

    QJsonDocument json_document = QJsonDocument::fromJson(import_file.readAll());
    import_file.close();
    const auto input = json_document.object();

    if(input.contains("_Type"))
    {
        const auto type = input["_Type"].toString();
        if(type == "raw")
            return importRawEFIData(input);

        if(type == "export")
            return importJSONEFIData(input);

        return showError(tr("Error importing boot configuration"), tr("Invalid _Type: %1").arg(input["_Type"].toString()));
    }

    return importJSONEFIData(input);
}

void EFIBootEditor::importJSONEFIData(const QJsonObject &input)
{
    showProgressBar(1, 2, tr("Importing boot configuration..."));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &type_fn, const QString &type_name, const auto &process_fn, const QString &name_prefix = "", bool optional = false)
    {
        const auto full_name = name_prefix + name;
        if(!root.contains(name))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        showProgressBar(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)...").arg(full_name));
        if(!(root[name].*type_fn)())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name).arg(type_name));
            return;
        }

        process_fn(root[name]);
    };

    // clang-format off
    process_entry(input, "Timeout", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->timeout_number->setValue(value.toInt());
    }, "", true);

    process_entry(input, "BootCurrent", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        current_boot = value.toInt();
    }, "", true);

    process_entry(input, "BootNext", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        next_boot = value.toInt();
    }, "", true);

    process_entry(input, "SecureBoot", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->secure_boot->setChecked(value.toInt());
    }, "", true);

    process_entry(input, "VendorKeys", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->vendor_keys->setChecked(value.toInt());
    }, "", true);

    process_entry(input, "SetupMode", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->setup_mode->setChecked(value.toInt());
    }, "", true);

    process_entry(input, "AuditMode", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->audit_mode->setChecked(value.toInt());
    }, "", true);

    process_entry(input, "DeployedMode", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        ui->deployed_mode->setChecked(value.toInt());
    }, "", true);

    process_entry(input, "OsIndicationsSupported", &QJsonValue::isArray, tr("array"), [&](const QJsonValue &jvalue)
    {
        uint64_t value = 0;
        const auto indications = jvalue.toArray();
        int i = -1;
        for(const auto indication: indications)
        {
            ++ i;
            const auto qname = QString("OsIndicationsSupported[%1]").arg(i);
            if(!indication.isString())
            {
                errors.push_back(tr("%1: %2 expected").arg(qname, tr("string")));
                continue;
            }

            if(indication == "BOOT_TO_FW_UI")
                value |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
            else if(indication == "TIMESTAMP_REVOCATION")
                value |= EFIBoot::EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION;
            else if(indication == "FILE_CAPSULE_DELIVERY_SUPPORTED")
                value |= EFIBoot::EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED;
            else if(indication == "FMP_CAPSULE_SUPPORTED")
                value |= EFIBoot::EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED;
            else if(indication == "CAPSULE_RESULT_VAR_SUPPORTED")
                value |= EFIBoot::EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED;
            else if(indication == "START_OS_RECOVERY")
                value |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;
            else if(indication == "START_PLATFORM_RECOVERY")
                value |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;
            else if(indication == "JSON_CONFIG_DATA_REFRESH")
                value |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;
            else
            {
                errors.push_back(tr("%1: unknown os indication").arg(qname));
                continue;
            }

            setupOsIndicationsSupport(value);
        }
    }, "", true);

    process_entry(input, "OsIndications", &QJsonValue::isArray, tr("array"), [&](const QJsonValue &jvalue)
    {
        uint64_t value = 0;
        const auto indications = jvalue.toArray();
        int i = -1;
        for(const auto indication: indications)
        {
            ++i;
            const auto qname = QString("OsIndications[%1]").arg(i);
            if(!indication.isString())
            {
                errors.push_back(tr("%1: %2 expected").arg(qname, tr("string")));
                continue;
            }

            if(indication == "BOOT_TO_FW_UI")
                value |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
            else if(indication == "START_OS_RECOVERY")
                value |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;
            else if(indication == "START_PLATFORM_RECOVERY")
                value |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;
            else if(indication == "JSON_CONFIG_DATA_REFRESH")
                value |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;
            else
            {
                errors.push_back(tr("%1: unknown os indication").arg(qname));
                continue;
            }
        }

        setupOsIndications(value);
    }, "", true);
    // clang-format on

    for(auto [prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        std::unordered_set<uint16_t> ordered_entry;

        // clang-format off
        process_entry(input, order_name, &QJsonValue::isArray, tr("array"), [&](const QJsonValue &value)
        {
            int i = -1;
            const auto boot_order = value.toArray();
            for(const auto index: boot_order)
            {
                ++i;
                if(!index.isDouble())
                {
                    const auto qname = QString("%1[%2]").arg(order_name, i);
                    errors.push_back(tr("%1: %2 expected").arg(qname, tr("number")));
                    continue;
                }

                const uint16_t idx = static_cast<uint16_t>(index.toInt());
                order.push_back(idx);
                ordered_entry.insert(idx);
            }
        }, "", true);

        process_entry(input, prefix, &QJsonValue::isObject, tr("object"), [&](const QJsonValue &root)
        {
            const QString full_prefix = QString("%1/").arg(prefix);
            const auto entries = root.toObject();
            const auto keys = entries.keys();
            for(const auto &name: keys)
            {
                bool success = false;
                const uint16_t index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
                if(!success)
                {
                    errors.push_back(tr("%1: %2 expected").arg(full_prefix + name, tr("hexadecimal number")));
                    continue;
                }

                if(ordered_entry.count(index))
                    continue;

                order.push_back(index);
                ordered_entry.insert(index);
            }

            model.clear();
            for(const auto &index: order)
            {
                const auto qname = toHex(index, 4, "");
                // clang-format off
                process_entry(entries, qname, &QJsonValue::isObject, tr("object"), [&](const QJsonValue &value)
                {
                    auto entry = BootEntry::fromJSON(value.toObject());
                    if(!entry)
                    {
                        errors.push_back(tr("%1: failed parsing").arg(full_prefix + qname));
                        return;
                    }

                    entry->index = index;
                    if(prefix == "Boot")
                    {
                        entry->is_current_boot = current_boot == static_cast<int>(index);
                        entry->is_next_boot = next_boot == static_cast<int>(index);
                    }
                    model.appendRow(*entry);
                }, full_prefix);
            }
        }, "", true);
        // clang-format on
    }

    // clang-format off
    process_entry(input, "Apple", &QJsonValue::isObject, tr("object"), [&](const QJsonValue &root)
    {
        const auto entries = root.toObject();
        process_entry(entries, "boot-args", &QJsonValue::isString, tr("string"), [&](const QJsonValue &value)
        {
            ui->boot_args_text->setText(value.toString());
        }, "Apple", true);
    }, "", true);
    // clang-format on

    if(!errors.isEmpty())
        showError(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::importRawEFIData(const QJsonObject &input)
{
    showProgressBar(1, 2, tr("Importing boot configuration..."));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &deserialize_fn, const auto &process_fn, const QString &name_prefix = "", bool optional = false)
    {
        const auto full_name = name_prefix + name;
        if(!root.contains(name))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        showProgressBar(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)...").arg(full_name));
        if(!root[name].isObject())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name, tr("object")));
            return;
        }

        const auto obj = root[name].toObject();
        if(!obj["raw_data"].isString() || !obj["efi_attributes"].isDouble())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name).arg(tr("object(raw_data: string, efi_attributes: number)")));
            return;
        }

        const auto raw_data = QByteArray::fromBase64(obj["raw_data"].toString().toUtf8());
        const auto value = deserialize_fn(raw_data.constData(), static_cast<size_t>(raw_data.size()));
        if(!value)
        {
            errors.push_back(tr("%1: failed deserialization").arg(full_name + "/raw_data"));
            return;
        }

        process_fn(*value, static_cast<uint32_t>(obj["efi_attributes"].toInt()));
    };

    // clang-format off
    process_entry(input, "Timeout", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        ui->timeout_number->setValue(value);
    }, "", true);

    process_entry(input, "BootCurrent", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        current_boot = value;
    }, "", true);

    process_entry(input, "BootNext", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        next_boot = value;
    }, "", true);

    process_entry(input, "SecureBoot", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->secure_boot->setChecked(value);
    }, "", true);

    process_entry(input, "VendorKeys", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->vendor_keys->setChecked(value);
    }, "", true);

    process_entry(input, "SetupMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->setup_mode->setChecked(value);
    }, "", true);

    process_entry(input, "AuditMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->audit_mode->setChecked(value);
    }, "", true);

    process_entry(input, "DeployedMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
    {
        ui->deployed_mode->setChecked(value);
    }, "", true);

    process_entry(input, "OsIndicationsSupported", EFIBoot::deserialize<uint64_t>, [&](const uint64_t &value, const auto &)
    {
        setupOsIndicationsSupport(value);
    }, "", true);

    process_entry(input, "OsIndications", EFIBoot::deserialize<uint64_t>, [&](const uint64_t &value, const auto &)
    {
        setupOsIndications(value);
    }, "", true);
    // clang-format on

    for(auto [prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;
        const QString order_name = QString("%1Order").arg(prefix);

        std::vector<uint16_t> order;
        std::unordered_set<uint16_t> ordered_entry;

        // clang-format off
        process_entry(input, order_name, EFIBoot::deserialize_list<uint16_t>, [&](const std::vector<uint16_t> &value, const auto &)
        {
            order = value;
            for(const uint16_t index: order)
                ordered_entry.insert(index);
        }, "", true);
        // clang-format on

        if(!input[prefix].isObject())
        {
            errors.push_back(tr("%1: %2 expected").arg(prefix, tr("object")));
            continue;
        }

        const QString full_prefix = QString("%1/").arg(prefix);
        const auto entries = input[prefix].toObject();
        const auto keys = entries.keys();
        for(const auto &name: keys)
        {
            bool success = false;
            const uint16_t index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
            if(!success)
            {
                errors.push_back(tr("%1: %2 expected").arg(full_prefix + name, tr("hexadecimal number")));
                continue;
            }

            if(ordered_entry.count(index))
                continue;

            order.push_back(index);
            ordered_entry.insert(index);
        }

        model.clear();
        for(const auto &index: order)
        {
            const auto qname = toHex(index, 4, "");
            // clang-format off
            process_entry(entries, qname, EFIBoot::deserialize<EFIBoot::Load_option>, [&](const EFIBoot::Load_option &value, const uint32_t &efi_attributes)
            {
                // Translate STL to QTL
                auto entry = BootEntry::fromEFIBootLoadOption(value);
                entry.index = index;
                entry.efi_attributes = static_cast<uint32_t>(efi_attributes);
                if(prefix == "Boot")
                {
                    entry.is_current_boot = current_boot == static_cast<int>(index);
                    entry.is_next_boot = next_boot == static_cast<int>(index);
                }
                model.appendRow(entry);
            }, full_prefix);
            // clang-format on
        }
    }

    if(input.contains("Apple"))
    {
        if(!input["Apple"].isObject())
            errors.push_back(tr("%1: %2 expected").arg("Apple", tr("object")));

        else
        {
            const auto entries = input["Apple"].toObject();
            // clang-format off
            process_entry(entries, "boot-args", EFIBoot::deserialize<std::string>, [&](const std::string &value, const auto &)
            {
                ui->boot_args_text->setText(QString::fromStdString(value));
            }, "Apple", true);
            // clang-format on
        }
    }

    if(!errors.isEmpty())
        showError(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::export_()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save boot configuration dump"), "", tr("JSON documents (*.json)"));
    if(file_name.isEmpty())
        return;

    if(!file_name.endsWith(".json", Qt::CaseInsensitive))
        file_name += ".json";

    exportBootConfiguration(file_name);
}

void EFIBootEditor::exportBootConfiguration(const QString &file_name)
{
    showProgressBar(0, 1, tr("Exporting boot configuration..."));
    QFile export_file(file_name);
    if(!export_file.open(QIODevice::WriteOnly))
        return showError(tr("Error exporting boot configuration"), tr("Couldn't open selected file (%1).").arg(file_name));

    int current_boot = -1;
    int next_boot = -1;
    QJsonObject output;
    size_t step = 0;
    size_t total_steps = 6u
        + static_cast<size_t>(boot_entries_list_model.getEntries().size())
        + static_cast<size_t>(driver_entries_list_model.getEntries().size())
        + static_cast<size_t>(sysprep_entries_list_model.getEntries().size())
        + static_cast<size_t>(platform_recovery_entries_list_model.getEntries().size());

    for(auto [prefix, model]: BOOT_ENTRIES)
    {
        const QString order_name = QString("%1Order").arg(prefix);

        QJsonArray order;
        QJsonObject entries;
        QSet<uint16_t> saved;
        for(const auto &entry: model.getEntries())
        {
            const auto name = toHex(entry.index, 4, "");
            const auto full_name = QString("%1%2").arg(prefix, name);
            if(saved.contains(entry.index))
                return showError(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(full_name, entry.description));

            saved.insert(entry.index);
            if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
            {
                order.push_back(entry.index);
                if(prefix == "Boot")
                {
                    if(entry.is_current_boot)
                        current_boot = entry.index;

                    if(entry.is_next_boot)
                        next_boot = entry.index;
                }
            }

            showProgressBar(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)...").arg(full_name));
            entries[name] = entry.toJSON();
        }

        if(!entries.isEmpty())
            output[prefix] = entries;

        if(!order.isEmpty())
            output[order_name] = order;
    }

    if(current_boot != -1)
        output["BootCurrent"] = current_boot;

    if(next_boot != -1)
        output["BootNext"] = next_boot;

    output["Timeout"] = static_cast<uint16_t>(ui->timeout_number->value());
    output["SecureBoot"] = static_cast<uint8_t>(ui->secure_boot->isChecked());
    output["VendorKeys"] = static_cast<uint8_t>(ui->vendor_keys->isChecked());
    output["SetupMode"] = static_cast<uint8_t>(ui->setup_mode->isChecked());
    output["AuditMode"] = static_cast<uint8_t>(ui->audit_mode->isChecked());
    output["DeployedMode"] = static_cast<uint8_t>(ui->deployed_mode->isChecked());

    QJsonArray supported_indications;
    if(ui->boot_to_firmware_ui->isEnabled())
        supported_indications.push_back("BOOT_TO_FW_UI");

    if(ui->timestamp_based_revocation->isChecked())
        supported_indications.push_back("TIMESTAMP_REVOCATION");

    if(ui->file_capsule_support->isChecked())
        supported_indications.push_back("FILE_CAPSULE_DELIVERY_SUPPORTED");

    if(ui->fmp_capsule_support->isChecked())
        supported_indications.push_back("FMP_CAPSULE_SUPPORTED");

    if(ui->capsule_reporting_support->isChecked())
        supported_indications.push_back("CAPSULE_RESULT_VAR_SUPPORTED");

    if(ui->start_os_recovery->isEnabled())
        supported_indications.push_back("START_OS_RECOVERY");

    if(ui->start_platform_recovery->isEnabled())
        supported_indications.push_back("START_PLATFORM_RECOVERY");

    if(ui->collect_current_config->isEnabled())
        supported_indications.push_back("JSON_CONFIG_DATA_REFRESH");

    if(!supported_indications.isEmpty())
        output["OsIndicationsSupported"] = supported_indications;

    QJsonArray indications;
    if(ui->boot_to_firmware_ui->isChecked())
        indications.push_back("BOOT_TO_FW_UI");

    if(ui->start_os_recovery->isChecked())
        indications.push_back("START_OS_RECOVERY");

    if(ui->start_platform_recovery->isChecked())
        indications.push_back("START_PLATFORM_RECOVERY");

    if(ui->collect_current_config->isChecked())
        indications.push_back("JSON_CONFIG_DATA_REFRESH");

    if(!indications.isEmpty())
        output["OsIndications"] = indications;

    // Apple
    const auto boot_args = ui->boot_args_text->text();
    if(!boot_args.isEmpty())
    {
        QJsonObject apple;
        apple["boot-args"] = boot_args;
        output["Apple"] = apple;
    }

    QJsonDocument json_document(output);
    export_file.write(json_document.toJson());
    export_file.close();

    hideProgressBar();
}

void EFIBootEditor::dump()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save raw EFI dump"), "", tr("JSON documents (*.json)"));
    if(file_name.isEmpty())
        return;

    if(!file_name.endsWith(".json", Qt::CaseInsensitive))
        file_name += ".json";

    dumpRawEFIData(file_name);
}

void EFIBootEditor::dumpRawEFIData(const QString &file_name)
{
    showProgressBar(0, 1, tr("Exporting boot configuration..."));
    QFile dump_file(file_name);
    if(!dump_file.open(QIODevice::WriteOnly))
        return showError(tr("Error dumping raw EFI data"), tr("Couldn't open selected file (%1).").arg(file_name));

    QJsonObject output;
    output["_Type"] = "raw";
    // clang-format off
    const auto name_to_guid = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view) { return guid == EFIBoot::efi_guid_global; },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching EFI Boot Manager entries...")); });
    // clang-format on

    QStringList errors;
    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;
    auto process_entry = [&](QJsonObject &root, const QString &key, std::tstring tname = _T(""), bool optional = false)
    {
        if(tname.empty())
            tname = QStringToStdTString(key);

        const auto qname = QStringFromStdTString(tname);
        if(!name_to_guid.count(tname))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(qname));

            return;
        }

        showProgressBar(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)...").arg(qname));
        const auto variable = EFIBoot::get_variable<EFIBoot::Raw_data>(name_to_guid.at(tname), tname);
        if(!variable)
        {
            errors.push_back(tr("%1: failed deserialization").arg(qname));
            return;
        }

        const auto &[value, attributes] = *variable;
        QJsonObject obj;
        obj["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
        obj["efi_attributes"] = static_cast<int>(attributes);
        root[key] = obj;
    };

    for(auto [prefix, model]: BOOT_ENTRIES)
    {
        (void)model;
        const QString order_name = QString("%1Order").arg(prefix);

        process_entry(output, order_name, _T(""), true);

        QJsonObject entries;
        for(const auto &[tname_, guid]: name_to_guid)
        {
            (void)guid;
            if(!is_bootentry(tname_, QStringToStdTString(prefix)))
                continue;

            const auto suffix = tname_.substr(static_cast<size_t>(prefix.size()));
            process_entry(entries, QStringFromStdTString(suffix), tname_);
        }

        if(!entries.isEmpty())
            output[prefix] = entries;
    }

    for(const auto &key: std::vector<const char *>{"Timeout", "BootCurrent", "BootNext", "SecureBoot", "VendorKeys", "SetupMode", "AuditMode", "DeployedMode", "OsIndicationsSupported", "OsIndications"})
        process_entry(output, key, _T(""), true);

    // Apple
    const auto boot_args = EFIBoot::get_variable<EFIBoot::Raw_data>(EFIBoot::efi_guid_apple, _T("boot-args"));
    if(boot_args)
    {
        QJsonObject apple;
        const auto &[value, attributes] = *boot_args;
        QJsonObject obj;
        obj["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
        obj["efi_attributes"] = static_cast<int>(attributes);
        apple["boot-args"] = obj;
        output["Apple"] = apple;
    }

    QJsonDocument json_document(output);
    dump_file.write(json_document.toJson());
    dump_file.close();

    if(!errors.isEmpty())
        showError(tr("Error dumping raw EFI data"), tr("Failed to dump some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::showAboutBox()
{
    auto *about = new QMessageBox(QMessageBox::Information,
        tr("About %1").arg(qApp->applicationName()),
        QString("<h1>%1</h1>"
                "<p><b>%2</b></p>"
                "<p>%3</p>")
            .arg(qApp->applicationName(), QCoreApplication::applicationVersion(), PROJECT_DESCRIPTION),
        QMessageBox::Close,
        this);
    about->setIconPixmap(QIcon::fromTheme("preferences-system").pixmap(128, 128));
    about->setInformativeText(tr("<p><a href='%1'>Website</a></p>"
                                 "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"
                                 "<p>License: <a href='https://www.gnu.org/licenses/lgpl.html'>GNU LGPL Version 3</a></p>"
                                 "<p>On Linux uses <a href='https://github.com/rhboot/efivar'>efivar</a> for EFI variables access.</p>"
                                 "<p>Uses Tango Icons as fallback icons.</p>")
                                  .arg(PROJECT_HOMEPAGE_URL));
    QObject::connect(about->button(QMessageBox::Close), &QAbstractButton::clicked, about, &QObject::deleteLater);
    QObject::connect(qApp, &QApplication::aboutToQuit, about, &QObject::deleteLater);
    about->show();
}

void EFIBootEditor::reorderBootEntries()
{
    auto [list, model] = currentBootEntryList();
    if(model.readonly)
        return;

    disableBootEntryEditor();

    uint16_t index = 0;
    for(int r = 0; r < model.rowCount(); ++r)
        model.changeData(model.index(r), [&index](BootEntry &entry)
            { entry.index = index++; return true; });

    enableBootEntryEditor(list.currentIndex());
}

void EFIBootEditor::removeCurrentBootEntry()
{
    auto [list, model] = currentBootEntryList();
    if(model.readonly)
        return;

    list.removeCurrentRow();
}

void EFIBootEditor::moveCurrentBootEntryUp()
{
    auto [list, model] = currentBootEntryList();
    if(model.readonly)
        return;

    list.moveCurrentRowUp();
}

void EFIBootEditor::moveCurrentBootEntryDown()
{
    auto [list, model] = currentBootEntryList();
    if(model.readonly)
        return;

    list.moveCurrentRowDown();
}

void EFIBootEditor::insertBootEntry()
{
    auto [list, model] = currentBootEntryList();
    if(model.readonly)
        return;

    list.insertRow();
}

std::tuple<BootEntryListView &, BootEntryListModel &> EFIBootEditor::getBootEntryList(int index)
{
    switch(static_cast<BootEntryType>(index))
    {
    case BootEntryType::BOOT:
        return {*ui->boot_entries_list, boot_entries_list_model};

    case BootEntryType::DRIVER:
        return {*ui->driver_entries_list, driver_entries_list_model};

    case BootEntryType::SYSPREP:
        return {*ui->sysprep_entries_list, sysprep_entries_list_model};

    case BootEntryType::PLATFORM_RECOVERY:
        return {*ui->platform_recovery_entries_list, platform_recovery_entries_list_model};
    }

    Q_UNREACHABLE();
    return {*ui->boot_entries_list, boot_entries_list_model};
}

std::tuple<BootEntryListView &, BootEntryListModel &> EFIBootEditor::currentBootEntryList()
{
    return getBootEntryList(ui->entries->currentIndex());
}

void EFIBootEditor::setupOsIndications(uint64_t value)
{
    ui->boot_to_firmware_ui->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI);
    ui->start_os_recovery->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY);
    ui->start_platform_recovery->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY);
    ui->collect_current_config->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH);
}

uint64_t EFIBootEditor::getOsIndications() const
{
    uint64_t indications = 0;
    if(ui->boot_to_firmware_ui->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;

    if(ui->start_os_recovery->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;

    if(ui->start_platform_recovery->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;

    if(ui->collect_current_config->isChecked())
        indications |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;

    return indications;
}

void EFIBootEditor::setupOsIndicationsSupport(uint64_t value)
{
    // Firmware features
    ui->timestamp_based_revocation->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION);
    ui->file_capsule_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED);
    ui->fmp_capsule_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED);
    ui->capsule_reporting_support->setChecked(value & EFIBoot::EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED);

    // Firmware actions
    ui->boot_to_firmware_ui->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI);
    ui->start_os_recovery->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY);
    ui->start_platform_recovery->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY);
    ui->collect_current_config->setEnabled(value & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH);
}

void EFIBootEditor::closeEvent(QCloseEvent *event)
{
    event->ignore();
    confirmation->setText(tr("Are you sure you want to quit?"));
    confirmation->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation->exec();
    if(confirmation->clickedButton() == confirmation->button(QMessageBox::Yes))
        event->accept();
}

void EFIBootEditor::showError(const QString &message, const QString &details)
{
    hideProgressBar();
    error->setText(message);
    error->setDetailedText(details);
    error->show();
}

template <class Receiver, typename Slot>
void EFIBootEditor::showConfirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot)
{
    confirmation->setText(message);
    confirmation->setStandardButtons(buttons);
    QObject::connect(confirmation->button(confirmation_button), &QAbstractButton::clicked, confirmation_context, confirmation_slot);
    confirmation->show();
}

void EFIBootEditor::showProgressBar(size_t step, size_t total, const QString &details)
{
    progress->setMaximum(static_cast<int>(total));
    progress->setLabelText(details);
    progress->setValue(static_cast<int>(step));
}

void EFIBootEditor::hideProgressBar()
{
    progress->reset();
}
