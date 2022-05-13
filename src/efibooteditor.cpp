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

EFIBootEditor::EFIBootEditor(QWidget *parent)
    : QMainWindow{parent}
    , ui{std::make_unique<Ui::EFIBootEditor>()}
    , confirmation{std::make_unique<QMessageBox>(QMessageBox::Question, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , error{std::make_unique<QMessageBox>(QMessageBox::Critical, qApp->applicationName(), "", QMessageBox::NoButton, this)}
    , progress{std::make_unique<QProgressDialog>("Working...", nullptr, 0, 0, this)}
{
    ui->setupUi(this);
    ui->entries_list->setModel(&entries_list_model);
    ui->entry_form->setBootEntryListModel(entries_list_model);
    progress->setWindowModality(Qt::WindowModal);
}

EFIBootEditor::~EFIBootEditor()
{
}

void EFIBootEditor::reload()
{
    show_confirmation(
        tr("Are you sure you want to reload the entries?<br/>ALL of your changes will be lost!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::resetBootConfiguration);
}

void EFIBootEditor::resetBootConfiguration()
{
    disableBootEntryEditor();
    show_progress_bar(0, 1, "Loading EFI Boot Manager entries...");
    int next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<unsigned long> ordered_entry;
    QStringList invalid_keys;

    auto name_to_guid = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name)
        { return guid == EFIBoot::efi_guid_global && (name.substr(0, 4) == _T("Boot") || name == _T("Timeout")); },
        [&](size_t step, size_t total)
        { show_progress_bar(step, total + 1u, "Searching EFI Boot Manager entries..."); });

    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;
    if(name_to_guid.count(_T("Timeout")))
    {
        show_progress_bar(step++, total_steps, "Processing EFI Boot Manager entries (Timeout)...");
        auto variable = EFIBoot::get_variable<uint16_t>(name_to_guid[_T("Timeout")], _T("Timeout"));
        if(!variable)
            invalid_keys.push_back("Timeout");
        else
        {
            const auto &[value, attributes] = *variable;
            ui->timeout_number->setValue(value);
        }
    }

    if(name_to_guid.count(_T("BootNext")))
    {
        show_progress_bar(step++, total_steps, "Processing EFI Boot Manager entries (BootNext)...");
        auto variable = EFIBoot::get_variable<int32_t>(name_to_guid[_T("BootNext")], _T("BootNext"));
        if(!variable)
            invalid_keys.push_back("BootNext");
        else
        {
            const auto &[value, attributes] = *variable;
            next_boot = value;
        }
    }

    if(name_to_guid.count(_T("BootOrder")))
    {
        show_progress_bar(step++, total_steps, "Processing EFI Boot Manager entries (BootOrder)...");
        auto variable = EFIBoot::get_list_variable<uint16_t>(name_to_guid[_T("BootOrder")], _T("BootOrder"));
        if(!variable)
            invalid_keys.push_back("BootOrder");
        else
        {
            const auto &[value, attributes] = *variable;
            order = value;

            for(const auto &index: order)
                ordered_entry.insert(index);
        }
    }

    // Add entries not in BootOrder at the end
    for(const auto &[name, guid]: name_to_guid)
    {
        if(name.length() != 8 || name.substr(0, 4) != _T("Boot"))
            continue;

        auto suffix = name.substr(4);
        if(!isxnumber(suffix))
            continue;

        auto index = std::stoul(suffix, nullptr, HEX_BASE);
        if(ordered_entry.count(index))
            continue;

        order.push_back(static_cast<uint16_t>(index));
        ordered_entry.insert(index);
    }

    entries_list_model.clear();
    for(const auto &index: order)
    {
        auto qname = QString("Boot%1").arg(index, 4, HEX_BASE, QChar('0'));
        auto name = QStringToStdTString(qname);
        show_progress_bar(step++, total_steps, QString("Processing EFI Boot Manager entries (%1)...").arg(qname));
        auto variable = EFIBoot::get_variable<EFIBoot::Load_option>(name_to_guid[name], name);

        if(!variable)
        {
            invalid_keys.push_back(qname);
            continue;
        }

        const auto &[value, attributes] = *variable;

        // Translate STL to QTL
        auto entry = BootEntry::fromEFIBootLoadOption(value);
        entry.efi_attributes = attributes;
        entry.is_next_boot = next_boot == static_cast<int>(index);
        entries_list_model.appendRow(entry);
    }

    if(!invalid_keys.isEmpty())
        show_error(tr("Error loading entries!"), tr("Couldn't deserialize keys: %1").arg(invalid_keys.join(", ")));

    hide_progress_bar();
}

void EFIBootEditor::enableBootEntryEditor(const QModelIndex &index)
{
    if(!index.isValid())
        return disableBootEntryEditor();

    auto item = index.data().value<const BootEntry *>();
    ui->entry_form->setItem(index, item);
}

void EFIBootEditor::disableBootEntryEditor()
{
    ui->entry_form->setItem({}, nullptr);
}

void EFIBootEditor::save()
{
    show_confirmation(
        tr("Are you sure you want to save?<br/>Your EFI configration will be overwritten!"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Yes,
        this,
        &EFIBootEditor::saveBootConfiguration);
}

void EFIBootEditor::saveBootConfiguration()
{
    show_progress_bar(0, 1, "Saving EFI Boot Manager entries...");
    int next_boot = -1;
    quint16 index = 0;
    std::vector<uint16_t> boot_order;

    auto old_entries = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name)
        { return guid == EFIBoot::efi_guid_global && name.length() == 8 && name.substr(0, 4) == _T("Boot") && isxnumber(name.substr(4)); },
        [&](size_t step, size_t total)
        { show_progress_bar(step, total + 1u, "Searching old EFI Boot Manager entries..."); });

    size_t step = 0;
    size_t total_steps = static_cast<size_t>(entries_list_model.getEntries().size()) + 1u;
    for(const auto &entry: entries_list_model.getEntries())
    {
        auto qname = QString("Boot%1").arg(index, 4, HEX_BASE, QChar('0'));
        show_progress_bar(step++, total_steps, QString("Saving EFI Boot Manager entries (%1)...").arg(qname));
        if((entry.attributes & EFIBoot::LOAD_OPTION_CATEGORY_MASK) == EFIBoot::LOAD_OPTION_CATEGORY_BOOT)
        {
            boot_order.push_back(index);
            if(entry.is_next_boot)
                next_boot = index;
        }

        std::tstring name = QStringToStdTString(qname);
        if(auto _entry = old_entries.find(name); _entry != old_entries.end())
            old_entries.erase(_entry);

        auto load_option = entry.toEFIBootLoadOption();
        if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, name, EFIBoot::Variable<EFIBoot::Load_option>{load_option, entry.efi_attributes}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
            return show_error(tr("Error saving entries!"), QString("Entry %1:\n").arg(index + 1) + QStringFromStdTString(EFIBoot::get_error_trace()));

        ++index;
    }

    step = 0;
    total_steps = old_entries.size() + 1u;
    for(const auto &[name, guid]: old_entries)
    {
        show_progress_bar(step++, total_steps, QString("Removing old EFI Boot Manager entries (%1)...").arg(QStringFromStdTString(name)));
        if(!EFIBoot::del_variable(guid, name))
            return show_error(tr("Error removing old entries!"), QStringFromStdTString(EFIBoot::get_error_trace()));
    }

    step = 0;
    total_steps = 3;
    // Save order
    show_progress_bar(step++, total_steps, "Saving EFI Boot Manager entries (BootOrder)...");
    if(!EFIBoot::set_list_variable(EFIBoot::efi_guid_global, _T("BootOrder"), EFIBoot::Variable<std::vector<uint16_t>>{boot_order, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return show_error(tr("Error saving boot order!"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Save next boot
    show_progress_bar(step++, total_steps, "Saving EFI Boot Manager entries (BootNext)...");
    if(next_boot != -1 && !EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("BootNext"), EFIBoot::Variable<int32_t>{next_boot, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return show_error(tr("Error saving next boot!"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Save timeout
    auto timeout = static_cast<uint16_t>(ui->timeout_number->value());
    show_progress_bar(step++, total_steps, "Saving EFI Boot Manager entries (Timeout)...");
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("Timeout"), EFIBoot::Variable<uint16_t>{timeout, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return show_error(tr("Error saving timeout!"), QStringFromStdTString(EFIBoot::get_error_trace()));

    hide_progress_bar();
}

void EFIBootEditor::import()
{
    auto file_name = QFileDialog::getOpenFileName(this, tr("Open Boot Configuration Dump"), "", tr("JSON Documents (*.json)"));
    if(!file_name.isEmpty())
        importBootConfiguration(file_name);
}

void EFIBootEditor::importBootConfiguration(const QString &file_name)
{
    disableBootEntryEditor();
    show_progress_bar(0, 1, "Importing boot configuration...");
    QFile import_file(file_name);
    if(!import_file.open(QIODevice::ReadOnly))
        return show_error(tr("Error importing boot configuration!"));

    QJsonDocument json_document = QJsonDocument::fromJson(import_file.readAll());
    const auto input = json_document.object();

    if(input.contains("_Type"))
    {
        const auto type = input["_Type"].toString();
        if(type == "raw")
            return importRawEFIData(input);

        if(type == "export")
            return importJSONEFIData(input);

        return show_error(tr("Error importing boot configuration!"), tr("Invalid _Type: %1").arg(input["_Type"].toString()));
    }

    return importJSONEFIData(input);
}

void EFIBootEditor::importJSONEFIData(const QJsonObject &input)
{
    show_progress_bar(1, 2, "Importing boot configuration...");
    int next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<unsigned long> ordered_entry;

    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;
    if(input.contains("Timeout"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (Timeout)...");
        if(!input["Timeout"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Timeout: number expected"));

        ui->timeout_number->setValue(input["Timeout"].toInt());
    }

    if(input.contains("BootNext"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (BootNext)...");
        if(!input["BootNext"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootNext: number expected"));

        next_boot = input["BootNext"].toInt();
    }

    if(input.contains("BootOrder"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (BootOrder)...");
        if(!input["BootOrder"].isArray())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootOrder: array expected"));

        int i = 0;
        const auto boot_order = input["BootOrder"].toArray();
        for(const auto &index: boot_order)
        {
            if(!index.isDouble())
                return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootOrder[%1]: number expected").arg(i));

            auto idx = static_cast<uint16_t>(index.toInt());
            order.push_back(idx);
            ordered_entry.insert(idx);
            ++i;
        }
    }

    if(!input["Boot"].isObject())
        return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot: object expected"));

    const auto boot = input["Boot"].toObject();
    const auto keys = boot.keys();
    for(const auto &name: keys)
    {
        bool success = false;
        auto index = name.toULong(&success, HEX_BASE);
        if(!success)
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot/%1: hexadecimal number expected").arg(name));

        if(ordered_entry.count(index))
            continue;

        order.push_back(static_cast<uint16_t>(index));
        ordered_entry.insert(index);
    }

    entries_list_model.clear();
    for(const auto &index: order)
    {
        auto name = QString("%1").arg(index, 4, HEX_BASE, QChar('0'));
        show_progress_bar(step++, total_steps, QString("Importing EFI Boot Manager entries (Boot%1)...").arg(name));
        auto entry = BootEntry::fromJSON(boot[name].toObject());
        if(!entry)
        {
            entries_list_model.clear();
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse boot entry %1").arg(name));
        }

        entry->is_next_boot = next_boot == static_cast<int>(index);
        entries_list_model.appendRow(*entry);
    }

    hide_progress_bar();
}

void EFIBootEditor::importRawEFIData(const QJsonObject &input)
{
    show_progress_bar(1, 2, "Importing boot configuration...");
    int next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<unsigned long> ordered_entry;
    QStringList invalid_keys;

    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;
    if(input.contains("Timeout"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (Timeout)...");
        if(!input["Timeout"].isObject())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Timeout: object expected"));

        auto timeout = input["Timeout"].toObject();
        if(!timeout["raw_data"].isString() || !timeout["efi_attributes"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Timeout: object(raw_data: string, efi_attributes: number) expected"));

        auto raw_data = QByteArray::fromBase64(timeout["raw_data"].toString().toUtf8());
        auto value = EFIBoot::deserialize<uint16_t>(raw_data.constData(), static_cast<size_t>(raw_data.size()));
        if(!value)
            invalid_keys.push_back("Timeout/raw_data");
        else
            ui->timeout_number->setValue(*value);
    }

    if(input.contains("BootNext"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (BootNext)...");
        if(!input["BootNext"].isObject())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootNext: object expected"));

        auto boot_next = input["BootNext"].toObject();
        if(!boot_next["raw_data"].isString() || !boot_next["efi_attributes"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootNext: object(raw_data: string, efi_attributes: number) expected"));

        auto raw_data = QByteArray::fromBase64(boot_next["raw_data"].toString().toUtf8());
        auto value = EFIBoot::deserialize<int32_t>(raw_data.constData(), static_cast<size_t>(raw_data.size()));
        if(!value)
            invalid_keys.push_back("BootNext/raw_data");
        else
            next_boot = *value;
    }

    if(input.contains("BootOrder"))
    {
        show_progress_bar(step++, total_steps, "Importing EFI Boot Manager entries (BootOrder)...");
        if(!input["BootOrder"].isObject())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootOrder: object expected"));

        auto boot_order = input["BootOrder"].toObject();
        if(!boot_order["raw_data"].isString() || !boot_order["efi_attributes"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse BootOrder: object(raw_data: string, efi_attributes: number) expected"));

        auto raw_data = QByteArray::fromBase64(boot_order["raw_data"].toString().toUtf8());
        auto value = EFIBoot::deserialize_list<uint16_t>(raw_data.constData(), static_cast<size_t>(raw_data.size()));
        if(!value)
            invalid_keys.push_back("BootOrder/raw_data");
        else
        {
            order = *value;
            for(auto index: order)
                ordered_entry.insert(index);
        }
    }

    if(!input["Boot"].isObject())
        return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot: object expected"));

    const auto boot = input["Boot"].toObject();
    const auto keys = boot.keys();
    for(const auto &name: keys)
    {
        bool success = true;
        auto index = name.toULong(&success, HEX_BASE);
        if(!success)
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot/%1: hexadecimal number expected").arg(name));

        if(ordered_entry.count(index))
            continue;

        order.push_back(static_cast<uint16_t>(index));
        ordered_entry.insert(index);
    }

    entries_list_model.clear();
    for(const auto &index: order)
    {
        auto name = QString("%1").arg(index, 4, HEX_BASE, QChar('0'));
        show_progress_bar(step++, total_steps, QString("Importing EFI Boot Manager entries (Boot%1)...").arg(name));
        if(!boot[name].isObject())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot/%1: object expected").arg(name));

        auto boot_entry = boot[name].toObject();
        if(!boot_entry["raw_data"].isString() || !boot_entry["efi_attributes"].isDouble())
            return show_error(tr("Error importing boot configuration!"), tr("Couldn't parse Boot/%1: object(raw_data: string, efi_attributes: number) expected").arg(name));

        auto raw_data = QByteArray::fromBase64(boot_entry["raw_data"].toString().toUtf8());
        auto value = EFIBoot::deserialize<EFIBoot::Load_option>(raw_data.constData(), static_cast<size_t>(raw_data.size()));
        if(!value)
            invalid_keys.push_back(QString("Boot/%1/raw_data").arg(name));
        else
        {
            // Translate STL to QTL
            auto entry = BootEntry::fromEFIBootLoadOption(*value);
            entry.efi_attributes = static_cast<quint32>(boot_entry["efi_attributes"].toInt());
            entry.is_next_boot = next_boot == static_cast<int>(index);
            entries_list_model.appendRow(entry);
        }
    }

    if(!invalid_keys.isEmpty())
        show_error(tr("Error importing boot configuration!"), tr("Couldn't deserialize keys: %1").arg(invalid_keys.join(", ")));

    hide_progress_bar();
}

void EFIBootEditor::export_()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save Boot Configuration Dump"), "", tr("JSON documents (*.json)"));
    if(!file_name.isEmpty())
        exportBootConfiguration(file_name);
}

void EFIBootEditor::exportBootConfiguration(const QString &file_name)
{
    show_progress_bar(0, 1, "Exporting boot configuration...");
    int next_boot = -1;
    quint16 index = 0;
    QJsonArray boot_order;
    QJsonObject output;
    QJsonObject entries;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(entries_list_model.getEntries().size()) + 1u;
    for(const auto &entry: entries_list_model.getEntries())
    {
        if((entry.attributes & EFIBoot::LOAD_OPTION_CATEGORY_MASK) == EFIBoot::LOAD_OPTION_CATEGORY_BOOT)
        {
            boot_order.push_back(index);
            if(entry.is_next_boot)
                next_boot = index;
        }

        auto name = QString("%1").arg(index, 4, HEX_BASE, QChar('0'));
        show_progress_bar(step++, total_steps, QString("Exporing EFI Boot Manager entries (Boot%1)...").arg(name));
        auto load_option = entry.toJSON();
        entries[name] = load_option;
        ++index;
    }

    output["Boot"] = entries;
    output["BootOrder"] = boot_order;
    if(next_boot != -1)
        output["BootNext"] = next_boot;

    auto timeout = static_cast<uint16_t>(ui->timeout_number->value());
    output["Timeout"] = timeout;

    QFile export_file(file_name);
    if(!export_file.open(QIODevice::WriteOnly))
        return show_error(tr("Error exporting boot configuration!"));

    QJsonDocument json_document(output);
    export_file.write(json_document.toJson());
    hide_progress_bar();
}

void EFIBootEditor::dump()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save Raw EFI Dump"), "", tr("JSON documents (*.json)"));
    if(!file_name.isEmpty())
        dumpRawEFIData(file_name);
}

void EFIBootEditor::dumpRawEFIData(const QString &file_name)
{
    show_progress_bar(0, 1, "Exporting boot configuration...");
    QJsonObject output;
    output["_Type"] = "raw";
    const auto name_to_guid = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name)
        { return guid == EFIBoot::efi_guid_global && (name.substr(0, 4) == _T("Boot") || name == _T("Timeout")); },
        [&](size_t step, size_t total)
        { show_progress_bar(step, total + 1u, "Searching EFI Boot Manager entries..."); });

    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;
    if(name_to_guid.count(_T("Timeout")))
    {
        show_progress_bar(step++, total_steps, "Exporting EFI Boot Manager entries (Timeout)...");
        const auto variable = EFIBoot::get_variable<EFIBoot::Raw_data>(name_to_guid.at(_T("Timeout")), _T("Timeout"));
        if(variable)
        {
            const auto &[value, attributes] = *variable;
            QJsonObject timeout;
            timeout["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
            timeout["efi_attributes"] = static_cast<int>(attributes);
            output["Timeout"] = timeout;
        }
    }

    if(name_to_guid.count(_T("BootNext")))
    {
        show_progress_bar(step++, total_steps, "Exporting EFI Boot Manager entries (BootNext)...");
        const auto variable = EFIBoot::get_variable<EFIBoot::Raw_data>(name_to_guid.at(_T("BootNext")), _T("BootNext"));
        if(variable)
        {
            const auto &[value, attributes] = *variable;
            QJsonObject boot_next;
            boot_next["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
            boot_next["efi_attributes"] = static_cast<int>(attributes);
            output["BootNext"] = boot_next;
        }
    }

    if(name_to_guid.count(_T("BootOrder")))
    {
        show_progress_bar(step++, total_steps, "Exporting EFI Boot Manager entries (BootOrder)...");
        const auto variable = EFIBoot::get_list_variable<EFIBoot::Raw_data>(name_to_guid.at(_T("BootOrder")), _T("BootOrder"));
        if(variable)
        {
            const auto &[value, attributes] = *variable;
            QJsonObject boot_order;
            boot_order["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
            boot_order["efi_attributes"] = static_cast<int>(attributes);
            output["BootOrder"] = boot_order;
        }
    }

    QJsonObject entries;
    for(const auto &[name, guid]: name_to_guid)
    {
        if(name.length() != 8 || name.substr(0, 4) != _T("Boot"))
            continue;

        auto suffix = name.substr(4);
        if(!isxnumber(suffix))
            continue;

        QString qname = QStringFromStdTString(suffix);
        show_progress_bar(step++, total_steps, QString("Exporting EFI Boot Manager entries (Boot%1)...").arg(qname));
        const auto variable = EFIBoot::get_variable<EFIBoot::Raw_data>(name_to_guid.at(name), name);
        if(!variable)
            return show_error(tr("Error dumping raw EFI data!"));

        const auto &[value, attributes] = *variable;
        QJsonObject boot;
        boot["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
        boot["efi_attributes"] = static_cast<int>(attributes);
        entries[qname] = boot;
    }

    output["Boot"] = entries;

    QFile dump_file(file_name);
    if(!dump_file.open(QIODevice::WriteOnly))
        return show_error(tr("Error dumping raw EFI data!"));

    QJsonDocument json_document(output);
    dump_file.write(json_document.toJson());
    hide_progress_bar();
}

void EFIBootEditor::showAboutBox()
{
    auto *about = new QMessageBox(QMessageBox::Information,
        qApp->applicationName(),
        QString("<h1>EFI Boot Editor</h1><p><b>%1</b></p>").arg(QCoreApplication::applicationVersion()),
        QMessageBox::Close,
        this);
    about->setInformativeText(tr(
        "<p>License: <a href='http://www.gnu.org/licenses/lgpl.html'>GNU LGPL Version 3</a></p>"
        "<p>On *nix uses <a href='https://github.com/rhboot/efivar'>efivar</a> for EFI variables access.</p>"
        "<p>Uses Tango Icons as fallback icons</p>"
        "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"));
    QObject::connect(about->button(QMessageBox::Close), &QAbstractButton::clicked, about, &QObject::deleteLater);
    QObject::connect(qApp, &QApplication::aboutToQuit, about, &QObject::deleteLater);
    about->show();
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

void EFIBootEditor::show_error(const QString &message, const QString &details)
{
    error->setText(message);
    error->setDetailedText(details);
    error->show();
}

template <class Receiver, typename Slot>
void EFIBootEditor::show_confirmation(const QString &message, const QMessageBox::StandardButtons &buttons, const QMessageBox::StandardButton &confirmation_button, Receiver confirmation_context, Slot confirmation_slot)
{
    confirmation->setText(message);
    confirmation->setStandardButtons(buttons);
    QObject::connect(confirmation->button(confirmation_button), &QAbstractButton::clicked, confirmation_context, confirmation_slot);
    confirmation->show();
}

void EFIBootEditor::show_progress_bar(size_t step, size_t total, const QString &details)
{
    progress->setMaximum(static_cast<int>(total));
    progress->setLabelText(details);
    progress->setValue(static_cast<int>(step));
}

void EFIBootEditor::hide_progress_bar()
{
    progress->reset();
}
