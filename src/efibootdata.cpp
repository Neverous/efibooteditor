// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "efibootdata.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <unordered_set>

#include "commands.h"

static bool is_bootentry(const tstring_view &name, const tstring_view &prefix)
{
    if(name.length() != prefix.length() + 4 || name.substr(0, prefix.length()) != prefix)
        return false;

    auto suffix = name.substr(prefix.length());
    return isxnumber(suffix);
}

EFIBootData::EFIBootData(QObject *parent)
    : QObject{parent}
{
    boot_entries_list_model.setUndoStack(undo_stack);
    driver_entries_list_model.setUndoStack(undo_stack);
    sysprep_entries_list_model.setUndoStack(undo_stack);
    platform_recovery_entries_list_model.setUndoStack(undo_stack);
}

QUndoStack *EFIBootData::getUndoStack() const
{
    return undo_stack;
}

void EFIBootData::setUndoStack(QUndoStack *undo_stack_)
{
    undo_stack = undo_stack_;
    boot_entries_list_model.setUndoStack(undo_stack);
    driver_entries_list_model.setUndoStack(undo_stack);
    sysprep_entries_list_model.setUndoStack(undo_stack);
    platform_recovery_entries_list_model.setUndoStack(undo_stack);
}

void EFIBootData::clear()
{
    boot_entries_list_model.clear();
    driver_entries_list_model.clear();
    sysprep_entries_list_model.clear();
    platform_recovery_entries_list_model.clear();

    setTimeout(0);
    setSecureBoot(false);
    setVendorKeys(false);
    setSetupMode(false);
    setAuditMode(false);
    setDeployedMode(false);
    setBootOptionSupport(0);
    setOsIndicationsSupported(0);
    setOsIndications(0);
    setAppleBootArgs("");

    if(undo_stack)
        undo_stack->clear();
}

void EFIBootData::reload()
{
    emit progress(0, 1, tr("Loading EFI Boot Manager entries…"));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;
    auto save_error = [&](const QString &error)
    {
        errors.push_back(error);
    };

    const auto variables = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const tstring_view)
        {
            return guid == EFIBoot::efi_guid_global;
        },
        [&](size_t step, size_t total)
        {
            emit progress(step, total + 1u, tr("Searching EFI Boot Manager entries…"));
        });

    if(!variables)
    {
        emit error(tr("Couldn't load EFI Boot Manager variables"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    const auto name_to_guid = *variables;
    if(name_to_guid.size() == 0)
        errors.push_back(tr("Couldn't find any EFI Boot Manager variables"));

    size_t step = 1;
    const size_t total_steps = name_to_guid.size() + 1u;

    auto process_entry = [&](const auto &name, const auto &read_fn, const auto &process_fn, const auto &error_fn, bool optional = false)
    {
        const auto tname = QStringToStdTString(name);
        emit progress(step++, total_steps, tr("Processing EFI Boot Manager entries (%1)…").arg(name));
        if(!name_to_guid.count(tname))
        {
            if(!optional)
                error_fn(tr("%1: not found").arg(name));

            return;
        }

        const auto variable = read_fn(name_to_guid.at(tname), tname);
        if(!variable)
        {
            error_fn(tr("%1: failed deserialization").arg(name));
            return;
        }

        const auto &[value, attributes] = *variable;
        process_fn(value, attributes);
    };

    process_entry(
        "Timeout", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
        { setTimeout(value); },
        save_error,
        true);

    process_entry(
        "BootCurrent", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
        { current_boot = value; },
        save_error,
        true);

    process_entry(
        "BootNext", EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
        { next_boot = value; },
        save_error,
        true);

    process_entry(
        "SecureBoot", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
        { setSecureBoot(value); },
        save_error,
        true);

    process_entry(
        "VendorKeys", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
        { setVendorKeys(value); },
        save_error,
        true);

    process_entry(
        "SetupMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
        { setSetupMode(value); },
        save_error,
        true);

    process_entry(
        "AuditMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
        { setAuditMode(value); },
        save_error,
        true);

    process_entry(
        "DeployedMode", EFIBoot::get_variable<uint8_t>, [&](const uint8_t &value, const auto &)
        { setDeployedMode(value); },
        save_error,
        true);

    process_entry(
        "BootOptionSupport", EFIBoot::get_variable<uint32_t>, [&](const uint32_t &value, const auto &)
        { setBootOptionSupport(value); },
        save_error,
        true);

    process_entry(
        "OsIndicationsSupported", EFIBoot::get_variable<uint64_t>, [&](const uint64_t &value, const auto &)
        { setOsIndicationsSupported(value); },
        save_error,
        true);

    process_entry(
        "OsIndications", EFIBoot::get_variable<uint64_t>, [&](const uint64_t &value, const auto &)
        { setOsIndications(value); },
        save_error,
        true);

    for(const auto &[prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        std::unordered_set<unsigned long> ordered_entry;

        process_entry(
            order_name, EFIBoot::get_list_variable<uint16_t>,
            [&](const std::vector<uint16_t> &value, const auto &)
            {
                order = value;
                for(const auto &index: order)
                    ordered_entry.insert(index);
            },
            save_error,
            true);

        // Add entries not in BootOrder at the end
        for(const auto &[tname, guid]: name_to_guid)
        {
            Q_UNUSED(guid)
            if(!is_bootentry(tname, QStringToStdTString(prefix)))
                continue;

            const auto index = static_cast<uint16_t>(std::stoul(tname.substr(static_cast<size_t>(prefix.size())), nullptr, HEX_BASE));
            if(ordered_entry.count(index))
                continue;

            order.push_back(index);
            ordered_entry.insert(index);
        }

        model.clear();
        for(const auto &index: order)
        {
            const auto qname = toHex(index, 4, prefix);

            process_entry(
                qname, EFIBoot::get_variable<EFIBoot::Load_option>,
                [&](const EFIBoot::Load_option &value, const uint32_t &attributes)
                {
                    // Translate STL to QTL
                    auto entry = BootEntry::fromEFIBootLoadOption(value);
                    entry.index = index;
                    entry.efi_attributes = attributes;
                    if(model.options & BootEntryListModel::Option::IsBoot)
                    {
                        entry.is_current_boot = current_boot == static_cast<int>(index);
                        entry.is_next_boot = next_boot == static_cast<int>(index);
                    }
                    model.appendRow(entry);
                },
                [&](const QString &error)
                {
                    errors.push_back(error);
                    auto entry = BootEntry::fromError(error);
                    entry.index = index;
                    if(model.options & BootEntryListModel::Option::IsBoot)
                    {
                        entry.is_current_boot = current_boot == static_cast<int>(index);
                        entry.is_next_boot = next_boot == static_cast<int>(index);
                    }
                    model.appendRow(entry);
                });
        }
    }

    // Apple
    emit progress(step++, total_steps, tr("Processing EFI Boot Manager entries (%1)…").arg("Apple/boot-args"));
    if(const auto boot_args = EFIBoot::get_variable<std::string>(EFIBoot::efi_guid_apple, _T("boot-args")); boot_args)
    {
        const auto &[value, attributes] = *boot_args;
        Q_UNUSED(attributes)
        setAppleBootArgs(QString::fromStdString(value));
    }

    if(!errors.isEmpty())
        emit error(tr("Error loading entries"), tr("Failed to load some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    emit done();
}

void EFIBootData::save()
{
    emit progress(0, 1, tr("Saving EFI Boot Manager entries…"));
    int32_t next_boot = -1;

    auto variables = EFIBoot::get_variables(
        [&](const EFIBoot::efi_guid_t &guid, const tstring_view tname)
        {
            if(guid != EFIBoot::efi_guid_global)
                return false;

            for(const auto &[prefix, model]: BOOT_ENTRIES)
            {
                if(model.options & BootEntryListModel::Option::ReadOnly)
                    continue;

                if(is_bootentry(tname, QStringToStdTString(prefix)))
                    return true;
            }

            return false;
        },
        [&](size_t step, size_t total)
        {
            emit progress(step, total + 1u, tr("Searching old EFI Boot Manager entries…"));
        });

    if(!variables)
    {
        emit error(tr("Couldn't load EFI Boot Manager variables"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    auto old_entries = *variables;
    size_t step = 1;
    size_t total_steps = 4u // remember to update when adding static vars
        + static_cast<size_t>(boot_entries_list_model.getEntries().size())
        + static_cast<size_t>(driver_entries_list_model.getEntries().size())
        + static_cast<size_t>(sysprep_entries_list_model.getEntries().size())
        + 3u
        + old_entries.size()
        + 1u;

    // Save entries
    for(const auto &[prefix, model]: BOOT_ENTRIES)
    {
        if(model.options & BootEntryListModel::Option::ReadOnly)
            continue;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        QSet<uint16_t> saved;
        for(const auto &entry: model.getEntries())
        {
            const auto qname = toHex(entry.index, 4, prefix);
            if(saved.contains(entry.index))
            {
                emit error(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(qname, entry.description));
                return;
            }

            saved.insert(entry.index);
            emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg(qname));
            if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
            {
                order.push_back(entry.index);
                if(entry.is_next_boot)
                    next_boot = entry.index;
            }

            const tstring tname = QStringToStdTString(qname);
            if(auto _entry = old_entries.find(tname); _entry != old_entries.end())
                old_entries.erase(_entry);

            if(entry.is_error)
                continue;

            const auto load_option = entry.toEFIBootLoadOption();
            if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, tname, EFIBoot::Variable<EFIBoot::Load_option>{load_option, entry.efi_attributes}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
            {
                emit error(tr("Error saving %1").arg(qname), QStringFromStdTString(EFIBoot::get_error_trace()));
                return;
            }
        }

        // Save order
        if(order.empty())
        {
            emit progress(step++, total_steps, tr("Removing EFI Boot Manager entries (%1)…").arg(order_name));
            if(EFIBoot::get_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name)) && !EFIBoot::del_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name)))
            {
                emit error(tr("Error removing %1").arg(order_name), QStringFromStdTString(EFIBoot::get_error_trace()));
                return;
            }

            // EFIBoot::get_variable above might've thrown an error, we don't care about it
            EFIBoot::error_clear();
        }
        else
        {
            emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg(order_name));
            if(!EFIBoot::set_list_variable(EFIBoot::efi_guid_global, QStringToStdTString(order_name), EFIBoot::Variable<std::vector<uint16_t>>{order, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
            {
                emit error(tr("Error saving %1").arg(order_name), QStringFromStdTString(EFIBoot::get_error_trace()));
                return;
            }
        }
    }

    // Remove old entries
    for(const auto &[tname, guid]: old_entries)
    {
        emit progress(step++, total_steps, tr("Removing old EFI Boot Manager entries (%1)…").arg(QStringFromStdTString(tname)));
        if(!EFIBoot::del_variable(guid, tname))
        {
            emit error(tr("Error removing %1").arg(QStringFromStdTString(tname)), QStringFromStdTString(EFIBoot::get_error_trace()));
            return;
        }
    }

    // Save next boot
    emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg("BootNext"));
    if(next_boot != -1 && !EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("BootNext"), EFIBoot::Variable<uint16_t>{static_cast<uint16_t>(next_boot), EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
    {
        emit error(tr("Error saving %1").arg("BootNext"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    // Save timeout
    emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg("Timeout"));
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("Timeout"), EFIBoot::Variable<uint16_t>{timeout, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
    {
        emit error(tr("Error saving %1").arg("Timeout"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg("OsIndications"));
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("OsIndications"), EFIBoot::Variable<uint64_t>{indications, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
    {
        emit error(tr("Error saving %1").arg("OsIndications"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    // Apple
    if(apple_boot_args.isEmpty())
    {
        emit progress(step++, total_steps, tr("Removing EFI Boot Manager entries (%1)…").arg("Apple/boot-args"));
        if(EFIBoot::get_variable(EFIBoot::efi_guid_apple, _T("boot-args")) && !EFIBoot::del_variable(EFIBoot::efi_guid_apple, _T("boot-args")))
        {
            emit error(tr("Error removing %1").arg("Apple/boot-args"), QStringFromStdTString(EFIBoot::get_error_trace()));
            return;
        }

        // EFIBoot::get_variable above might've thrown an error, we don't care about it
        EFIBoot::error_clear();
    }
    else
    {
        emit progress(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)…").arg("Apple/boot-args"));
        if(!EFIBoot::set_variable(EFIBoot::efi_guid_apple, _T("boot-args"), EFIBoot::Variable<std::string>{apple_boot_args.toStdString(), EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        {
            emit error(tr("Error saving %1").arg("Apple/boot-args"), QStringFromStdTString(EFIBoot::get_error_trace()));
            return;
        }
    }

    if(undo_stack)
        undo_stack->clear();

    emit done();
}

void EFIBootData::import_(const QString &file_name)
{
    emit progress(0, 1, tr("Importing boot configuration…"));
    QFile import_file(file_name);
    if(!import_file.open(QIODevice::ReadOnly))
    {
        emit error(tr("Error importing boot configuration"), tr("Couldn't open selected file (%1).").arg(file_name));
        return;
    }

    QJsonParseError json_error{};
    QJsonDocument json_document = QJsonDocument::fromJson(import_file.readAll(), &json_error);
    import_file.close();
    if(json_document.isNull())
    {
        emit error(tr("Error importing boot configuration"), tr("Parser failed: %1").arg(json_error.errorString()));
        return;
    }

    const auto input = json_document.object();
    if(input.contains("_Type"))
    {
        const auto type = input["_Type"].toString();
        if(type == "raw")
            return importRawEFIData(input);

        if(type == "export")
            return importJSONEFIData(input);

        emit error(tr("Error importing boot configuration"), tr("Invalid _Type: %1").arg(input["_Type"].toString()));
        return;
    }

    return importJSONEFIData(input);
}

void EFIBootData::export_(const QString &file_name)
{
    emit progress(0, 1, tr("Exporting boot configuration…"));
    QFile export_file(file_name);
    if(!export_file.open(QIODevice::WriteOnly))
    {
        emit error(tr("Error exporting boot configuration"), tr("Couldn't open selected file (%1): %2.").arg(file_name, export_file.errorString()));
        return;
    }

    int current_boot = -1;
    int next_boot = -1;
    QJsonObject output;
    size_t step = 1;
    size_t total_steps = 11u // remember to update when adding static vars
        + static_cast<size_t>(boot_entries_list_model.getEntries().size())
        + static_cast<size_t>(driver_entries_list_model.getEntries().size())
        + static_cast<size_t>(sysprep_entries_list_model.getEntries().size())
        + static_cast<size_t>(platform_recovery_entries_list_model.getEntries().size())
        + 8u + 1u;

    auto progress_fn = [&](const QString &name)
    {
        emit progress(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)…").arg(name));
    };

    progress_fn("Timeout");
    output["Timeout"] = timeout;

    progress_fn("SecureBoot");
    output["SecureBoot"] = secure_boot != 0;

    progress_fn("VendorKeys");
    output["VendorKeys"] = vendor_keys != 0;

    progress_fn("SetupMode");
    output["SetupMode"] = setup_mode != 0;

    progress_fn("AuditMode");
    output["AuditMode"] = audit_mode != 0;

    progress_fn("DeployedMode");
    output["DeployedMode"] = deployed_mode != 0;

    if(boot_option_support)
    {
        QJsonObject obj;
        QJsonArray caps;

        if(boot_option_support & EFIBoot::EFI_BOOT_OPTION_SUPPORT_KEY)
            caps.push_back("KEY");

        if(boot_option_support & EFIBoot::EFI_BOOT_OPTION_SUPPORT_APP)
            caps.push_back("APP");

        if(boot_option_support & EFIBoot::EFI_BOOT_OPTION_SUPPORT_SYSPREP)
            caps.push_back("SYSPREP");

        obj["capabilities"] = caps;
        obj["key_count"] = static_cast<int>((boot_option_support & EFIBoot::EFI_BOOT_OPTION_SUPPORT_COUNT) >> 8);
        output["BootOptionSupport"] = obj;
    }

    {
        QJsonArray arr;
        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI)
            arr.push_back("BOOT_TO_FW_UI");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION)
            arr.push_back("TIMESTAMP_REVOCATION");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED)
            arr.push_back("FILE_CAPSULE_DELIVERY_SUPPORTED");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED)
            arr.push_back("FMP_CAPSULE_SUPPORTED");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED)
            arr.push_back("CAPSULE_RESULT_VAR_SUPPORTED");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY)
            arr.push_back("START_OS_RECOVERY");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY)
            arr.push_back("START_PLATFORM_RECOVERY");

        if(supported_indications & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH)
            arr.push_back("JSON_CONFIG_DATA_REFRESH");

        progress_fn("OsIndicationsSupported");
        if(!arr.isEmpty())
            output["OsIndicationsSupported"] = arr;
    }

    {
        QJsonArray arr;
        if(indications & EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI)
            arr.push_back("BOOT_TO_FW_UI");

        if(indications & EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY)
            arr.push_back("START_OS_RECOVERY");

        if(indications & EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY)
            arr.push_back("START_PLATFORM_RECOVERY");

        if(indications & EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH)
            arr.push_back("JSON_CONFIG_DATA_REFRESH");

        progress_fn("OsIndications");
        if(!arr.isEmpty())
            output["OsIndications"] = arr;
    }

    for(const auto &[prefix, model]: BOOT_ENTRIES)
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
            {
                emit error(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(full_name, entry.description));
                return;
            }

            progress_fn(full_name);
            saved.insert(entry.index);
            if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
            {
                order.push_back(entry.index);
                if(model.options & BootEntryListModel::Option::IsBoot)
                {
                    if(entry.is_current_boot)
                        current_boot = entry.index;

                    if(entry.is_next_boot)
                        next_boot = entry.index;
                }
            }

            if(!entry.is_error)
                entries[name] = entry.toJSON();
        }

        progress_fn(order_name);
        if(!order.isEmpty())
            output[order_name] = order;

        progress_fn(prefix);
        if(!entries.isEmpty())
            output[prefix] = entries;
    }

    progress_fn("BootCurrent");
    if(current_boot != -1)
        output["BootCurrent"] = current_boot;

    progress_fn("BootNext");
    if(next_boot != -1)
        output["BootNext"] = next_boot;

    // Apple
    progress_fn("Apple/boot-args");
    if(!apple_boot_args.isEmpty())
    {
        QJsonObject apple;
        apple["boot-args"] = apple_boot_args;
        output["Apple"] = apple;
    }

    if(QJsonDocument json_document(output); !export_file.write(json_document.toJson()))
    {
        emit error(tr("Error exporting boot configuration"), tr("Couldn't write into file (%1): %2.").arg(file_name, export_file.errorString()));
        return;
    }

    export_file.close();

    emit done();
}

void EFIBootData::dump(const QString &file_name)
{
    emit progress(0, 1, tr("Exporting boot configuration…"));
    QFile dump_file(file_name);
    if(!dump_file.open(QIODevice::WriteOnly))
    {
        emit error(tr("Error dumping raw EFI data"), tr("Couldn't open selected file (%1).").arg(file_name));
        return;
    }

    QJsonObject output;
    output["_Type"] = "raw";
    const auto variables = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const tstring_view)
        {
            return guid == EFIBoot::efi_guid_global;
        },
        [&](size_t step, size_t total)
        {
            emit progress(step, total + 1u, tr("Searching EFI Boot Manager entries…"));
        });

    if(!variables)
    {
        emit error(tr("Couldn't load EFI Boot Manager variables"), QStringFromStdTString(EFIBoot::get_error_trace()));
        return;
    }

    const auto name_to_guid = *variables;
    QStringList errors;
    if(name_to_guid.size() == 0)
        errors.push_back(tr("Couldn't find any EFI Boot Manager variables"));

    size_t step = 1;
    const size_t total_steps = name_to_guid.size() + 1u;
    auto process_entry = [&](QJsonObject &root, const QString &key, tstring tname = _T(""), bool optional = false)
    {
        if(tname.empty())
            tname = QStringToStdTString(key);

        const auto qname = QStringFromStdTString(tname);
        emit progress(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)…").arg(qname));
        if(!name_to_guid.count(tname))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(qname));

            return;
        }

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

    for(const auto &key: std::vector<const char *>{"Timeout", "BootCurrent", "BootNext", "SecureBoot", "VendorKeys", "SetupMode", "AuditMode", "DeployedMode", "BootOptionSupport", "OsIndicationsSupported", "OsIndications"})
        process_entry(output, key, _T(""), true);

    for(const auto &[prefix, model]: BOOT_ENTRIES)
    {
        Q_UNUSED(model)
        const QString order_name = QString("%1Order").arg(prefix);

        process_entry(output, order_name, _T(""), true);

        QJsonObject entries;
        for(const auto &[tname_, guid]: name_to_guid)
        {
            Q_UNUSED(guid)
            if(!is_bootentry(tname_, QStringToStdTString(prefix)))
                continue;

            const auto suffix = tname_.substr(static_cast<size_t>(prefix.size()));
            process_entry(entries, QStringFromStdTString(suffix), tname_);
        }

        if(!entries.isEmpty())
            output[prefix] = entries;
    }

    // Apple
    emit progress(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)…").arg("Apple/boot-args"));
    if(const auto boot_args = EFIBoot::get_variable<EFIBoot::Raw_data>(EFIBoot::efi_guid_apple, _T("boot-args")); boot_args)
    {
        QJsonObject apple;
        const auto &[value, attributes] = *boot_args;
        QJsonObject obj;
        obj["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
        obj["efi_attributes"] = static_cast<int>(attributes);
        apple["boot-args"] = obj;
        output["Apple"] = apple;
    }

    if(QJsonDocument json_document(output); !dump_file.write(json_document.toJson()))
    {
        emit error(tr("Error dumping raw EFI data"), tr("Couldn't write into file (%1): %2.").arg(file_name, dump_file.errorString()));
        return;
    }

    dump_file.close();
    if(!errors.isEmpty())
        emit error(tr("Error dumping raw EFI data"), tr("Failed to dump some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    emit done();
}

void EFIBootData::setTimeout(uint16_t value)
{
    if(timeout == value)
        return;

    auto command = new SetEFIBootDataValueCommand{*this, tr("Timeout"), &EFIBootData::timeout, &EFIBootData::timeoutChanged, value};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void EFIBootData::setAppleBootArgs(const QString &text)
{
    if(apple_boot_args == text)
        return;

    auto command = new SetEFIBootDataValueCommand{*this, tr("Apple boot-args"), &EFIBootData::apple_boot_args, &EFIBootData::appleBootArgsChanged, text};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void EFIBootData::setOsIndications(uint64_t value)
{
    if(indications == value)
        return;

    auto command = new SetEFIBootDataValueCommand{*this, tr("Firmware actions"), &EFIBootData::indications, &EFIBootData::osIndicationsChanged, value};
    if(!undo_stack)
    {
        command->redo();
        delete command;
        return;
    }

    undo_stack->push(command);
}

void EFIBootData::setSecureBoot(bool enabled)
{
    if(secure_boot == enabled)
        return;

    secure_boot = enabled;
    emit secureBootChanged(secure_boot);
}

void EFIBootData::setVendorKeys(bool enabled)
{
    if(vendor_keys == enabled)
        return;

    vendor_keys = enabled;
    emit vendorKeysChanged(vendor_keys);
}

void EFIBootData::setSetupMode(bool enabled)
{
    if(setup_mode == enabled)
        return;

    setup_mode = enabled;
    emit setupModeChanged(setup_mode);
}

void EFIBootData::setAuditMode(bool enabled)
{
    if(audit_mode == enabled)
        return;

    audit_mode = enabled;
    emit auditModeChanged(audit_mode);
}

void EFIBootData::setDeployedMode(bool enabled)
{
    if(deployed_mode == enabled)
        return;

    deployed_mode = enabled;
    emit deployedModeChanged(deployed_mode);
}

void EFIBootData::setBootOptionSupport(uint32_t flags)
{
    if(boot_option_support == flags)
        return;

    boot_option_support = flags;
    emit bootOptionSupportChanged(boot_option_support);
}

void EFIBootData::setOsIndicationsSupported(uint64_t value)
{
    if(supported_indications == value)
        return;

    supported_indications = value;
    emit osIndicationsSupportedChanged(supported_indications);
}

void EFIBootData::importJSONEFIData(const QJsonObject &input)
{
    emit progress(0, 1, tr("Importing boot configuration from JSON…"));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;

    size_t step = 1;
    auto total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &type_fn, const QString &type_name, const auto &process_fn, const QString &name_prefix = "", bool optional = false)
    {
        const auto full_name = name_prefix + name;
        if(!root.contains(name))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        emit progress(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)…").arg(full_name));
        if(!(root[name].*type_fn)())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name).arg(type_name));
            return;
        }

        process_fn(root[name]);
    };

    process_entry(
        input, "Timeout", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
        { setTimeout(static_cast<uint16_t>(value.toInt())); },
        "", true);

    process_entry(
        input, "BootCurrent", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
        { current_boot = static_cast<int32_t>(value.toInt()); },
        "", true);

    process_entry(
        input, "BootNext", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
        { next_boot = static_cast<int32_t>(value.toInt()); },
        "", true);

    process_entry(
        input, "SecureBoot", &QJsonValue::isBool, tr("bool"), [&](const QJsonValue &value)
        { setSecureBoot(value.toBool()); },
        "", true);

    process_entry(
        input, "VendorKeys", &QJsonValue::isBool, tr("bool"), [&](const QJsonValue &value)
        { setVendorKeys(value.toBool()); },
        "", true);

    process_entry(
        input, "SetupMode", &QJsonValue::isBool, tr("bool"), [&](const QJsonValue &value)
        { setSetupMode(value.toBool()); },
        "", true);

    process_entry(
        input, "AuditMode", &QJsonValue::isBool, tr("bool"), [&](const QJsonValue &value)
        { setAuditMode(value.toBool()); },
        "", true);

    process_entry(
        input, "DeployedMode", &QJsonValue::isBool, tr("bool"), [&](const QJsonValue &value)
        { setDeployedMode(value.toBool()); },
        "", true);

    process_entry(
        input, "BootOptionSupport", &QJsonValue::isObject, tr("object"), [&](const QJsonValue &value)
        {
            uint32_t val = 0;
            const auto obj = value.toObject();
            if(obj["capabilities"].isArray())
            {
                const auto caps = obj["capabilities"].toArray();
                int i = -1;
                for(const auto cap: caps)
                {
                    ++i;
                    const auto qname = QString("BootOptionSupport/capabilities[%1]").arg(i);
                    if(!cap.isString())
                    {
                        errors.push_back(tr("%1: %2 expected").arg(qname, tr("string")));
                        continue;
                    }

                    if(cap == "KEY")
                        val |= EFIBoot::EFI_BOOT_OPTION_SUPPORT_KEY;
                    else if(cap == "APP")
                        val |= EFIBoot::EFI_BOOT_OPTION_SUPPORT_APP;
                    else if(cap == "SYSPREP")
                        val |= EFIBoot::EFI_BOOT_OPTION_SUPPORT_SYSPREP;
                    else
                    {
                        errors.push_back(tr("%1: unknown boot manager capability").arg(qname));
                        continue;
                    }
                }
            }

            if(obj["key_count"].isDouble())
                val |= (static_cast<uint32_t>(obj["key_count"].toInt()) << 8) & EFIBoot::EFI_BOOT_OPTION_SUPPORT_COUNT;

            setBootOptionSupport(val); },
        "", true);

    process_entry(
        input, "OsIndicationsSupported", &QJsonValue::isArray, tr("array"),
        [&](const QJsonValue &value)
        {
            uint64_t val = 0;
            const auto arr = value.toArray();
            int i = -1;
            for(const auto indication: arr)
            {
                ++i;
                const auto qname = QString("OsIndicationsSupported[%1]").arg(i);
                if(!indication.isString())
                {
                    errors.push_back(tr("%1: %2 expected").arg(qname, tr("string")));
                    continue;
                }

                if(indication == "BOOT_TO_FW_UI")
                    val |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
                else if(indication == "TIMESTAMP_REVOCATION")
                    val |= EFIBoot::EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION;
                else if(indication == "FILE_CAPSULE_DELIVERY_SUPPORTED")
                    val |= EFIBoot::EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED;
                else if(indication == "FMP_CAPSULE_SUPPORTED")
                    val |= EFIBoot::EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED;
                else if(indication == "CAPSULE_RESULT_VAR_SUPPORTED")
                    val |= EFIBoot::EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED;
                else if(indication == "START_OS_RECOVERY")
                    val |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;
                else if(indication == "START_PLATFORM_RECOVERY")
                    val |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;
                else if(indication == "JSON_CONFIG_DATA_REFRESH")
                    val |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;
                else
                {
                    errors.push_back(tr("%1: unknown os indication").arg(qname));
                    continue;
                }
            }

            setOsIndicationsSupported(val);
        },
        "", true);

    process_entry(
        input, "OsIndications", &QJsonValue::isArray, tr("array"),
        [&](const QJsonValue &value)
        {
            uint64_t val = 0;
            const auto arr = value.toArray();
            int i = -1;
            for(const auto indication: arr)
            {
                ++i;
                const auto qname = QString("OsIndications[%1]").arg(i);
                if(!indication.isString())
                {
                    errors.push_back(tr("%1: %2 expected").arg(qname, tr("string")));
                    continue;
                }

                if(indication == "BOOT_TO_FW_UI")
                    val |= EFIBoot::EFI_OS_INDICATIONS_BOOT_TO_FW_UI;
                else if(indication == "START_OS_RECOVERY")
                    val |= EFIBoot::EFI_OS_INDICATIONS_START_OS_RECOVERY;
                else if(indication == "START_PLATFORM_RECOVERY")
                    val |= EFIBoot::EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY;
                else if(indication == "JSON_CONFIG_DATA_REFRESH")
                    val |= EFIBoot::EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH;
                else
                {
                    errors.push_back(tr("%1: unknown os indication").arg(qname));
                    continue;
                }
            }

            setOsIndications(val);
        },
        "", true);

    for(const auto &[prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        std::unordered_set<uint16_t> ordered_entry;

        process_entry(
            input, order_name, &QJsonValue::isArray, tr("array"),
            [&](const QJsonValue &value)
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

                    const auto idx = static_cast<uint16_t>(index.toInt());
                    order.push_back(idx);
                    ordered_entry.insert(idx);
                }
            },
            "", true);

        process_entry(
            input, prefix, &QJsonValue::isObject, tr("object"),
            [&](const QJsonValue &root)
            {
                const QString full_prefix = QString("%1/").arg(prefix);
                const auto entries = root.toObject();
                total_steps += qMax(static_cast<size_t>(entries.size()), static_cast<size_t>(order.size()));
                const auto keys = entries.keys();
                for(const auto &name: keys)
                {
                    bool success = false;
                    const auto index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
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
                    process_entry(
                        entries, qname, &QJsonValue::isObject, tr("object"),
                        [&](const QJsonValue &value)
                        {
                            auto entry = BootEntry::fromJSON(value.toObject());
                            if(!entry)
                            {
                                errors.push_back(tr("%1: failed parsing").arg(full_prefix + qname));
                                return;
                            }

                            entry->index = index;
                            if(model.options & BootEntryListModel::Option::IsBoot)
                            {
                                entry->is_current_boot = current_boot == static_cast<int>(index);
                                entry->is_next_boot = next_boot == static_cast<int>(index);
                            }
                            model.appendRow(*entry);
                        },
                        full_prefix);
                }
            },
            "", order.empty());
    }

    process_entry(
        input, "Apple", &QJsonValue::isObject, tr("object"),
        [&](const QJsonValue &root)
        {
            const auto entries = root.toObject();
            process_entry(
                entries, "boot-args", &QJsonValue::isString, tr("string"), [&](const QJsonValue &value)
                { setAppleBootArgs(value.toString()); },
                "Apple", true);
        },
        "", true);

    if(!errors.isEmpty())
        emit error(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    emit done();
}

void EFIBootData::importRawEFIData(const QJsonObject &input)
{
    emit progress(0, 1, tr("Importing boot configuration from raw dump…"));
    int32_t current_boot = -1;
    int32_t next_boot = -1;
    QStringList errors;

    size_t step = 1;
    auto total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &deserialize_fn, const auto &process_fn, const QString &name_prefix = "", bool optional = false)
    {
        const auto full_name = name_prefix + name;
        if(!root.contains(name))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        emit progress(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)…").arg(full_name));
        if(!root[name].isObject())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name, tr("object")));
            return;
        }

        const auto obj = root[name].toObject();
        if(!obj["raw_data"].isString() || !obj["efi_attributes"].isDouble())
        {
            errors.push_back(tr("%1: %2 expected").arg(full_name).arg(
                //: Expected JSON structure, thrown as error description.
                //: raw_data and efi_attributes are field names in JSON file
                tr("object(raw_data: string, efi_attributes: number)")));
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

    process_entry(
        input, "Timeout", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
        { setTimeout(value); },
        "", true);

    process_entry(
        input, "BootCurrent", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
        { current_boot = value; },
        "", true);

    process_entry(
        input, "BootNext", EFIBoot::deserialize<uint16_t>, [&](const uint16_t &value, const auto &)
        { next_boot = value; },
        "", true);

    process_entry(
        input, "SecureBoot", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
        { setSecureBoot(value); },
        "", true);

    process_entry(
        input, "VendorKeys", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
        { setVendorKeys(value); },
        "", true);

    process_entry(
        input, "SetupMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
        { setSetupMode(value); },
        "", true);

    process_entry(
        input, "AuditMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
        { setAuditMode(value); },
        "", true);

    process_entry(
        input, "DeployedMode", EFIBoot::deserialize<uint8_t>, [&](const uint8_t &value, const auto &)
        { setDeployedMode(value); },
        "", true);

    process_entry(
        input, "BootOptionSupport", EFIBoot::deserialize<uint32_t>, [&](const uint32_t &value, const auto &)
        { setBootOptionSupport(value); },
        "", true);

    process_entry(
        input, "OsIndicationsSupported", EFIBoot::deserialize<uint64_t>, [&](const uint64_t &value, const auto &)
        { setOsIndicationsSupported(value); },
        "", true);

    process_entry(
        input, "OsIndications", EFIBoot::deserialize<uint64_t>, [&](const uint64_t &value, const auto &)
        { setOsIndications(value); },
        "", true);

    for(const auto &[prefix_, model_]: BOOT_ENTRIES)
    {
        // References to local bindings don't work in lambdas
        auto &model = model_;
        auto &prefix = prefix_;

        const QString order_name = QString("%1Order").arg(prefix);
        std::vector<uint16_t> order;
        std::unordered_set<uint16_t> ordered_entry;

        process_entry(
            input, order_name, EFIBoot::deserialize_list<uint16_t>,
            [&](const std::vector<uint16_t> &value, const auto &)
            {
                order = value;
                for(const uint16_t index: order)
                    ordered_entry.insert(index);
            },
            "", true);

        if(!input.contains(prefix))
        {
            if(!order.empty())
                errors.push_back(tr("%1: not found").arg(prefix));

            continue;
        }

        emit progress(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)…").arg(prefix));
        if(!input[prefix].isObject())
        {
            errors.push_back(tr("%1: %2 expected").arg(prefix, tr("object")));
            continue;
        }

        const QString full_prefix = QString("%1/").arg(prefix);
        const auto entries = input[prefix].toObject();
        total_steps += qMax(static_cast<size_t>(entries.size()), static_cast<size_t>(order.size()));
        const auto keys = entries.keys();
        for(const auto &name: keys)
        {
            bool success = false;
            const auto index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
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
            process_entry(
                entries, qname, EFIBoot::deserialize<EFIBoot::Load_option>,
                [&](const EFIBoot::Load_option &value, const uint32_t &efi_attributes)
                {
                    // Translate STL to QTL
                    auto entry = BootEntry::fromEFIBootLoadOption(value);
                    entry.index = index;
                    entry.efi_attributes = efi_attributes;
                    if(model.options & BootEntryListModel::Option::IsBoot)
                    {
                        entry.is_current_boot = current_boot == static_cast<int>(index);
                        entry.is_next_boot = next_boot == static_cast<int>(index);
                    }
                    model.appendRow(entry);
                },
                full_prefix);
        }
    }

    if(input.contains("Apple"))
    {
        if(!input["Apple"].isObject())
            errors.push_back(tr("%1: %2 expected").arg("Apple", tr("object")));

        else
        {
            const auto entries = input["Apple"].toObject();
            process_entry(
                entries, "boot-args", EFIBoot::deserialize<std::string>, [&](const std::string &value, const auto &)
                { setAppleBootArgs(QString::fromStdString(value)); },
                "Apple", true);
        }
    }

    if(!errors.isEmpty())
        emit error(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    emit done();
}
