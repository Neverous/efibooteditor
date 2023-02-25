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

static bool is_bootentry(const std::tstring_view &name)
{
    if(name.length() != 8 || name.substr(0, 4) != _T("Boot"))
        return false;

    auto suffix = name.substr(4);
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
    ui->entries_list->setModel(&entries_list_model);
    ui->entry_form->setBootEntryListModel(entries_list_model);
    progress->setWindowModality(Qt::WindowModal);
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
    showProgressBar(0, 1, tr("Loading EFI Boot Manager entries..."));
    int32_t next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<unsigned long> ordered_entry;
    QStringList errors;

    // clang-format off
    const auto name_to_guid = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name) { return guid == EFIBoot::efi_guid_global && (name.substr(0, 4) == _T("Boot") || name == _T("Timeout")); },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching EFI Boot Manager entries...")); });
    // clang-format on

    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;

    auto process_entry = [&](const auto &name, const auto &tname, const auto &read_fn, const auto &process_fn, bool optional = false)
    {
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
    process_entry("Timeout", _T("Timeout"), EFIBoot::get_variable<uint16_t>, [&](const uint16_t &value, const auto &)
    {
        ui->timeout_number->setValue(value);
    }, true);

    process_entry("BootNext", _T("BootNext"), EFIBoot::get_variable<int32_t>, [&](const int32_t &value, const auto &)
    {
        next_boot = value;
    }, true);

    process_entry("BootOrder", _T("BootOrder"), EFIBoot::get_list_variable<uint16_t>, [&](const std::vector<uint16_t> &value, const auto &)
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
        if(!is_bootentry(tname))
            continue;

        const uint16_t index = static_cast<uint16_t>(std::stoul(tname.substr(4), nullptr, HEX_BASE));
        if(ordered_entry.count(index))
            continue;

        order.push_back(index);
        ordered_entry.insert(index);
    }

    entries_list_model.clear();
    for(const auto &index: order)
    {
        const auto qname = toHex(index, 4, "Boot");

        // clang-format off
        process_entry(qname, QStringToStdTString(qname), EFIBoot::get_variable<EFIBoot::Load_option>, [&](const EFIBoot::Load_option &value, const uint32_t &attributes)
        {
            // Translate STL to QTL
            auto entry = BootEntry::fromEFIBootLoadOption(value);
            entry.index = index;
            entry.efi_attributes = attributes;
            entry.is_next_boot = next_boot == static_cast<int>(index);
            entries_list_model.appendRow(entry);
        });
        // clang-format on
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
    std::vector<uint16_t> boot_order;

    // clang-format off
    auto old_entries = EFIBoot::get_variables(
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name) { return guid == EFIBoot::efi_guid_global && is_bootentry(name); },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching old EFI Boot Manager entries...")); });
    // clang-format on

    size_t step = 0;
    size_t total_steps = static_cast<size_t>(entries_list_model.getEntries().size()) + 1u;
    QSet<quint16> saved;
    for(const auto &entry: entries_list_model.getEntries())
    {
        const auto qname = toHex(entry.index, 4, "Boot");
        if(saved.contains(entry.index))
            return showError(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(qname, entry.description));

        saved.insert(entry.index);
        showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg(qname));
        if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
        {
            boot_order.push_back(entry.index);
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

    step = 0;
    total_steps = old_entries.size() + 1u;
    for(const auto &[tname, guid]: old_entries)
    {
        showProgressBar(step++, total_steps, tr("Removing old EFI Boot Manager entries (%1)...").arg(QStringFromStdTString(tname)));
        if(!EFIBoot::del_variable(guid, tname))
            return showError(tr("Error removing %1").arg(QStringFromStdTString(tname)), QStringFromStdTString(EFIBoot::get_error_trace()));
    }

    step = 0;
    total_steps = 3;
    // Save order
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("BootOrder"));
    if(!EFIBoot::set_list_variable(EFIBoot::efi_guid_global, _T("BootOrder"), EFIBoot::Variable<std::vector<uint16_t>>{boot_order, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("BootOrder"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Save next boot
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("BootNext"));
    if(next_boot != -1 && !EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("BootNext"), EFIBoot::Variable<int32_t>{next_boot, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("BootNext"), QStringFromStdTString(EFIBoot::get_error_trace()));

    // Save timeout
    const auto timeout = static_cast<uint16_t>(ui->timeout_number->value());
    showProgressBar(step++, total_steps, tr("Saving EFI Boot Manager entries (%1)...").arg("Timeout"));
    if(!EFIBoot::set_variable(EFIBoot::efi_guid_global, _T("Timeout"), EFIBoot::Variable<uint16_t>{timeout, EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS}, EFIBoot::EFI_VARIABLE_MODE_DEFAULTS))
        return showError(tr("Error saving %1").arg("Timeout"), QStringFromStdTString(EFIBoot::get_error_trace()));

    hideProgressBar();
}

void EFIBootEditor::import()
{
    const auto file_name = QFileDialog::getOpenFileName(this, tr("Open Boot Configuration Dump"), "", tr("JSON Documents (*.json)"));
    if(!file_name.isEmpty())
        importBootConfiguration(file_name);
}

void EFIBootEditor::importBootConfiguration(const QString &file_name)
{
    disableBootEntryEditor();
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
    int32_t next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<uint16_t> ordered_entry;
    QStringList errors;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &type_fn, const QString &type_name, const auto &process_fn, const QString &prefix = "", bool optional = false)
    {
        const auto full_name = prefix + name;
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

    process_entry(input, "BootNext", &QJsonValue::isDouble, tr("number"), [&](const QJsonValue &value)
    {
        next_boot = value.toInt();
    }, "", true);

    process_entry(input, "BootOrder", &QJsonValue::isArray, tr("array"), [&](const QJsonValue &value)
    {
        int i = 0;
        const auto boot_order = value.toArray();
        for(const auto index: boot_order)
        {
            const auto &name = QString("BootOrder[%1]").arg(i);
            if(!index.isDouble())
            {
                errors.push_back(tr("%1: number expected").arg(name));
                continue;
            }

            const uint16_t idx = static_cast<uint16_t>(index.toInt());
            order.push_back(idx);
            ordered_entry.insert(idx);
            ++i;
        }
    }, "", true);
    // clang-format on

    if(!input["Boot"].isObject())
        errors.push_back(tr("%1: object expected").arg("Boot"));

    else
    {
        const QString prefix_ = "Boot/";
        const auto boot = input["Boot"].toObject();
        const auto keys = boot.keys();
        for(const auto &name: keys)
        {
            bool success = false;
            const uint16_t index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
            if(!success)
            {
                errors.push_back(tr("%1: hexadecimal number expected").arg(prefix_ + name));
                continue;
            }

            if(ordered_entry.count(index))
                continue;

            order.push_back(index);
            ordered_entry.insert(index);
        }

        entries_list_model.clear();
        for(const auto &index: order)
        {
            const auto qname = toHex(index, 4, "");
            // clang-format off
            process_entry(boot, qname, &QJsonValue::isObject, tr("object"), [&](const QJsonValue &value)
            {
                auto entry = BootEntry::fromJSON(value.toObject());
                if(!entry)
                {
                    errors.push_back(tr("%1: failed parsing").arg(prefix_ + qname));
                    return;
                }

                entry->index = index;
                entry->is_next_boot = next_boot == static_cast<int>(index);
                entries_list_model.appendRow(*entry);
            }, prefix_);
            // clang-format on
        }
    }

    if(!errors.isEmpty())
        showError(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::importRawEFIData(const QJsonObject &input)
{
    showProgressBar(1, 2, tr("Importing boot configuration..."));
    int32_t next_boot = -1;
    std::vector<uint16_t> order;
    std::unordered_set<uint16_t> ordered_entry;
    QStringList errors;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(input.size()) + 1u;

    auto process_entry = [&](const QJsonObject &root, const auto &name, const auto &deserialize_fn, const auto &process_fn, const QString &prefix = "", bool optional = false)
    {
        const auto full_name = prefix + name;
        if(!root.contains(name))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        showProgressBar(step++, total_steps, tr("Importing EFI Boot Manager entries (%1)...").arg(full_name));
        if(!root[name].isObject())
        {
            errors.push_back(tr("%1: object expected").arg(full_name));
            return;
        }

        const auto obj = root[name].toObject();
        if(!obj["raw_data"].isString() || !obj["efi_attributes"].isDouble())
        {
            errors.push_back(tr("%1: object(raw_data: string, efi_attributes: number) expected").arg(full_name));
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

    process_entry(input, "BootNext", EFIBoot::deserialize<int32_t>, [&](const int32_t &value, const auto &)
    {
        next_boot = value;
    },"", true);

    process_entry(input, "BootOrder", EFIBoot::deserialize_list<uint16_t>, [&](const std::vector<uint16_t> &value, const auto &)
    {
        order = value;
        for(const uint16_t index: order)
            ordered_entry.insert(index);
    },"", true);
    // clang-format on

    if(!input["Boot"].isObject())
        errors.push_back(tr("%1: object expected").arg("Boot"));

    else
    {
        const QString prefix_ = "Boot/";
        const auto boot = input["Boot"].toObject();
        const auto keys = boot.keys();
        for(const auto &name: keys)
        {
            bool success = false;
            const uint16_t index = static_cast<uint16_t>(name.toULong(&success, HEX_BASE));
            if(!success)
            {
                errors.push_back(tr("%1: hexadecimal number expected").arg(prefix_ + name));
                continue;
            }

            if(ordered_entry.count(index))
                continue;

            order.push_back(index);
            ordered_entry.insert(index);
        }

        entries_list_model.clear();
        for(const auto &index: order)
        {
            const auto qname = toHex(index, 4, "");
            // clang-format off
            process_entry(boot, qname, EFIBoot::deserialize<EFIBoot::Load_option>, [&](const EFIBoot::Load_option &value, const uint32_t &efi_attributes)
            {
                // Translate STL to QTL
                auto entry = BootEntry::fromEFIBootLoadOption(value);
                entry.index = index;
                entry.efi_attributes = static_cast<quint32>(efi_attributes);
                entry.is_next_boot = next_boot == static_cast<int>(index);
                entries_list_model.appendRow(entry);
            }, prefix_);
            // clang-format on
        }
    }

    if(!errors.isEmpty())
        showError(tr("Error importing boot configuration"), tr("Failed to import some EFI Boot Manager entries:\n\n  - %1").arg(errors.join("\n  - ")));

    hideProgressBar();
}

void EFIBootEditor::export_()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save Boot Configuration Dump"), "", tr("JSON documents (*.json)"));
    if(!file_name.isEmpty())
        exportBootConfiguration(file_name);
}

void EFIBootEditor::exportBootConfiguration(const QString &file_name)
{
    showProgressBar(0, 1, tr("Exporting boot configuration..."));
    QFile export_file(file_name);
    if(!export_file.open(QIODevice::WriteOnly))
        return showError(tr("Error exporting boot configuration"), tr("Couldn't open selected file (%1).").arg(file_name));

    int next_boot = -1;
    QJsonArray boot_order;
    QJsonObject output;
    QJsonObject entries;
    size_t step = 0;
    const size_t total_steps = static_cast<size_t>(entries_list_model.getEntries().size()) + 1u;
    QSet<quint16> saved;
    for(const auto &entry: entries_list_model.getEntries())
    {
        const auto name = toHex(entry.index, 4, "");
        const auto full_name = QString("Boot%1").arg(name);
        if(saved.contains(entry.index))
            return showError(tr("Error saving entries"), tr("Entry %1(%2): duplicated index!").arg(full_name, entry.description));

        saved.insert(entry.index);
        if((entry.attributes & EFIBoot::Load_option_attribute::CATEGORY_MASK) == EFIBoot::Load_option_attribute::CATEGORY_BOOT)
        {
            boot_order.push_back(entry.index);
            if(entry.is_next_boot)
                next_boot = entry.index;
        }

        showProgressBar(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)...").arg(full_name));
        const auto load_option = entry.toJSON();
        entries[name] = load_option;
    }

    output["Boot"] = entries;
    output["BootOrder"] = boot_order;
    if(next_boot != -1)
        output["BootNext"] = next_boot;

    output["Timeout"] = static_cast<uint16_t>(ui->timeout_number->value());

    QJsonDocument json_document(output);
    export_file.write(json_document.toJson());
    export_file.close();

    hideProgressBar();
}

void EFIBootEditor::dump()
{
    const QString file_name = QFileDialog::getSaveFileName(this, tr("Save Raw EFI Dump"), "", tr("JSON documents (*.json)"));
    if(!file_name.isEmpty())
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
        [](const EFIBoot::efi_guid_t &guid, const std::tstring_view name) { return guid == EFIBoot::efi_guid_global && (name.substr(0, 4) == _T("Boot") || name == _T("Timeout")); },
        [&](size_t step, size_t total) { showProgressBar(step, total + 1u, tr("Searching EFI Boot Manager entries...")); });
    // clang-format on

    QStringList errors;
    size_t step = 0;
    const size_t total_steps = name_to_guid.size() + 1u;
    auto process_entry = [&](QJsonObject &root, const auto &name, const auto &tname, const QString &prefix = "", bool optional = false)
    {
        const auto full_name = prefix + name;
        if(!name_to_guid.count(tname))
        {
            if(!optional)
                errors.push_back(tr("%1: not found").arg(full_name));

            return;
        }

        showProgressBar(step++, total_steps, tr("Exporting EFI Boot Manager entries (%1)...").arg(full_name));
        const auto variable = EFIBoot::get_variable<EFIBoot::Raw_data>(name_to_guid.at(tname), tname);
        if(!variable)
        {
            errors.push_back(tr("%1: failed deserialization").arg(full_name));
            return;
        }

        const auto &[value, attributes] = *variable;
        QJsonObject obj;
        obj["raw_data"] = QString(QByteArray::fromRawData(reinterpret_cast<const char *>(value.data()), static_cast<int>(value.size())).toBase64());
        obj["efi_attributes"] = static_cast<int>(attributes);
        root[name] = obj;
    };

    process_entry(output, "Timeout", _T("Timeout"), "", true);
    process_entry(output, "BootNext", _T("BootNext"), "", true);
    process_entry(output, "BootOrder", _T("BootOrder"), "", true);

    QJsonObject entries;
    for(const auto &[tname_, guid]: name_to_guid)
    {
        (void)guid;
        if(!is_bootentry(tname_))
            continue;

        const auto suffix = tname_.substr(4);
        const QString qname = QStringFromStdTString(suffix);
        process_entry(entries, qname, tname_, "Boot");
    }

    output["Boot"] = entries;

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
        qApp->applicationName(),
        QString("<h1>EFI Boot Editor</h1><p><b>%1</b></p>").arg(QCoreApplication::applicationVersion()),
        QMessageBox::Close,
        this);
    about->setInformativeText(tr(
        "<p>License: <a href='http://www.gnu.org/licenses/lgpl.html'>GNU LGPL Version 3</a></p>"
        "<p>On Linux uses <a href='https://github.com/rhboot/efivar'>efivar</a> for EFI variables access.</p>"
        "<p>Uses Tango Icons as fallback icons</p>"
        "<p>The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.</p>"));
    QObject::connect(about->button(QMessageBox::Close), &QAbstractButton::clicked, about, &QObject::deleteLater);
    QObject::connect(qApp, &QApplication::aboutToQuit, about, &QObject::deleteLater);
    about->show();
}

void EFIBootEditor::reorderBootEntries()
{
    disableBootEntryEditor();
    quint16 index = 0;
    for(int r = 0; r < entries_list_model.rowCount(); ++r)
        entries_list_model.changeData(entries_list_model.index(r), [&index](BootEntry &entry)
            { entry.index = index++; return true; });

    enableBootEntryEditor(ui->entries_list->currentIndex());
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
