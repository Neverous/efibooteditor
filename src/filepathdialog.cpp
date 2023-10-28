// SPDX-License-Identifier: LGPL-3.0-or-later
#include "filepathdialog.h"

#include "compat.h"
#include "driveinfo.h"
#include "form/ui_filepathdialog.h"

#include <QMessageBox>

QSize HorizontalTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
    if(type == QStyle::CT_TabBarTab)
        s.transpose();

    return s;
}

void HorizontalTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if(element != CE_TabBarTabLabel)
    {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option);
    if(!tab)
    {
        QProxyStyle::drawControl(element, option, painter, widget);
        return;
    }

    QStyleOptionTab opt(*tab);
    opt.shape = QTabBar::RoundedNorth;
    QProxyStyle::drawControl(element, &opt, painter, widget);
}

FilePathDialog::FilePathDialog(QWidget *parent)
    : QDialog(parent)
    , ui{std::make_unique<Ui::FilePathDialog>()}
{
    ui->setupUi(this);
    setFilePath(nullptr);
    ui->options->tabBar()->setStyle(&horizontal_tab_style);
}

FilePathDialog::~FilePathDialog()
{
}

void FilePathDialog::setReadOnly(bool readonly)
{
    for(auto &widget: findChildren<QLineEdit *>())
        widget->setReadOnly(readonly);

    for(auto &widget: findChildren<QPlainTextEdit *>())
        widget->setReadOnly(readonly);

    for(auto &widget: findChildren<QSpinBox *>())
        widget->setReadOnly(readonly);

    for(auto &widget: findChildren<QComboBox *>())
        widget->setDisabled(readonly);

    for(auto &widget: findChildren<QCheckBox *>())
        widget->setDisabled(readonly);

    for(auto &widget: findChildren<QLabel *>())
        widget->setDisabled(readonly);

    ui->disk_refresh->setDisabled(readonly);
    ui->options->tabBar()->setDisabled(readonly);
    ui->unknown_data_format_combo->setDisabled(false);
    ui->vendor_data_format_combo->setDisabled(false);
}

auto FilePathDialog::toFilePath() const -> File_path::ANY
{
    switch(static_cast<FormIndex>(ui->options->currentIndex()))
    {
    case FormIndex::PCI:
    {
        File_path::PCI pci;
        pci.function = static_cast<uint8_t>(ui->function_number->value());
        pci.device = static_cast<uint8_t>(ui->device_number->value());
        return pci;
    }
    case FormIndex::HID:
    {
        File_path::HID hid;
        hid.hid = ui->hid_text->text().toUInt(nullptr, HEX_BASE);
        hid.uid = ui->uid_text->text().toUInt(nullptr, HEX_BASE);
        return hid;
    }
    case FormIndex::USB:
    {
        File_path::USB usb;
        usb.parent_port_number = static_cast<uint8_t>(ui->parent_port_number->value());
        usb.interface = static_cast<uint8_t>(ui->interface_number->value());
        return usb;
    }
    case FormIndex::Vendor:
    {
        File_path::Vendor vendor;
        switch(static_cast<VendorTypeIndex>(ui->vendor_type_combo->currentIndex()))
        {
        case VendorTypeIndex::HW:
            vendor._type = EFIBoot::File_path::HWVendor::TYPE;
            break;

        case VendorTypeIndex::MSG:
            vendor._type = EFIBoot::File_path::MSGVendor::TYPE;
            break;

        case VendorTypeIndex::MEDIA:
            vendor._type = EFIBoot::File_path::MEDIAVendor::TYPE;
            break;
        }

        vendor.guid = QUuid::fromString(ui->vendor_guid_text->text());
        vendor.data = getVendorData(ui->vendor_data_format_combo->currentIndex());
        return vendor;
    }
    case FormIndex::MACAddress:
    {
        File_path::MACAddress mac_address;
        mac_address.address = ui->mac_address_text->text();
        mac_address.if_type = static_cast<uint8_t>(ui->if_type_number->value());
        return mac_address;
    }
    case FormIndex::IPv4:
    {
        File_path::IPv4 ipv4;
        ipv4.local_ip_address.setAddress(ui->ipv4_local_ip_text->text());
        ipv4.local_port = static_cast<uint16_t>(ui->ipv4_local_port_number->value());
        ipv4.remote_ip_address.setAddress(ui->ipv4_remote_ip_text->text());
        ipv4.remote_port = static_cast<uint16_t>(ui->ipv4_remote_port_number->value());
        ipv4.protocol = static_cast<uint16_t>(ui->ipv4_protocol_number->value());
        ipv4.static_ip_address = ui->ipv4_static->isChecked();
        ipv4.gateway_ip_address.setAddress(ui->ipv4_gateway_ip_text->text());
        ipv4.subnet_mask.setAddress(ui->ipv4_subnet_mask_text->text());
        return ipv4;
    }
    case FormIndex::IPv6:
    {
        File_path::IPv6 ipv6;
        ipv6.local_ip_address.setAddress(ui->ipv6_local_ip_text->text());
        ipv6.local_port = static_cast<uint16_t>(ui->ipv6_local_port_number->value());
        ipv6.remote_ip_address.setAddress(ui->ipv6_remote_ip_text->text());
        ipv6.remote_port = static_cast<uint16_t>(ui->ipv6_remote_port_number->value());
        ipv6.protocol = static_cast<uint16_t>(ui->ipv6_protocol_number->value());
        ipv6.ip_address_origin = static_cast<uint8_t>(ui->ipv6_origin_combo->currentIndex());
        ipv6.gateway_ip_address.setAddress(ui->ipv6_gateway_ip_text->text());
        ipv6.prefix_length = static_cast<uint8_t>(ui->ipv6_prefix_length_number->value());
        return ipv6;
    }
    case FormIndex::SATA:
    {
        File_path::SATA sata;
        sata.hba_port = static_cast<uint16_t>(ui->hba_port_number->value());
        sata.port_multiplier_port = static_cast<uint16_t>(ui->port_multiplier_port_number->value());
        sata.lun = static_cast<uint16_t>(ui->lun_number->value());
        return sata;
    }
    case FormIndex::HD:
    {
        File_path::HD hd;
        hd.signature_type = static_cast<decltype(hd.signature_type)>(ui->signature_type_combo->currentIndex());
        hd.partition_format = static_cast<decltype(hd.partition_format)>(hd.signature_type != EFIBoot::File_path::HD::SIGNATURE::NONE ? hd.signature_type : EFIBoot::File_path::HD::SIGNATURE::MBR);
        hd.partition_number = static_cast<uint32_t>(ui->partition_number->value());
        switch(hd.signature_type)
        {
        case EFIBoot::File_path::HD::SIGNATURE::GUID:
            hd.partition_signature = QUuid::fromString(ui->signature_text->text());
            break;

        case EFIBoot::File_path::HD::SIGNATURE::MBR:
            hd.partition_signature = QUuid{ui->signature_text->text().toUInt(nullptr, HEX_BASE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            break;

        case EFIBoot::File_path::HD::SIGNATURE::NONE:
            hd.partition_signature = QUuid{};
            break;
        }

        hd.partition_start = ui->start_text->text().toULongLong(nullptr, HEX_BASE);
        hd.partition_size = ui->size_text->text().toULongLong(nullptr, HEX_BASE);
        return hd;
    }
    case FormIndex::File:
    {
        File_path::File file;
        file.name = ui->filename_text->text();
        return file;
    }
    case FormIndex::FirmwareFile:
    {
        File_path::FirmwareFile firmware_file;
        firmware_file.name = QUuid::fromString(ui->firmware_file_text->text());
        return firmware_file;
    }
    case FormIndex::FirmwareVolume:
    {
        File_path::FirmwareVolume firmware_volume;
        firmware_volume.name = QUuid::fromString(ui->firmware_volume_text->text());
        return firmware_volume;
    }
    case FormIndex::BIOSBootSpecification:
    {
        File_path::BIOSBootSpecification bios_boot_specification;
        bios_boot_specification.device_type = ui->device_type_text->text().toUShort(nullptr, HEX_BASE);
        bios_boot_specification.status_flag = ui->status_flag_text->text().toUShort(nullptr, HEX_BASE);
        bios_boot_specification.description = ui->description_text->text();
        return bios_boot_specification;
    }
    case FormIndex::End:
    {
        File_path::End end;
        if(ui->end_subtype_combo->currentIndex() == 0)
            end._subtype = EFIBoot::File_path::End_instance::SUBTYPE;

        else
            end._subtype = EFIBoot::File_path::End_entire::SUBTYPE;

        return end;
    }
    case FormIndex::Unknown_:
    {
        File_path::Unknown unknown;
        unknown._type = static_cast<uint8_t>(ui->unknown_type_text->text().toUShort(nullptr, HEX_BASE));
        unknown._subtype = static_cast<uint8_t>(ui->unknown_subtype_text->text().toUShort(nullptr, HEX_BASE));
        unknown.data = getUnknownData(ui->unknown_data_format_combo->currentIndex());
        return unknown;
    }
    }

    return {};
}

void FilePathDialog::setFilePath(const File_path::ANY *_file_path)
{
    resetForms();
    if(!_file_path)
    {
        update();
        return;
    }

    struct Visitor
    {
        FilePathDialog *parent;
        explicit Visitor(FilePathDialog *parent_)
            : parent{parent_}
        {
        }

        void operator()(const File_path::PCI &pci) { parent->setPCIForm(pci); }
        void operator()(const File_path::HID &hid) { parent->setHIDForm(hid); }
        void operator()(const File_path::USB &usb) { parent->setUSBForm(usb); }
        void operator()(const File_path::Vendor &vendor) { parent->setVendorForm(vendor); }
        void operator()(const File_path::MACAddress &mac_address) { parent->setMACAddressForm(mac_address); }
        void operator()(const File_path::IPv4 &ipv4) { parent->setIPv4Form(ipv4); }
        void operator()(const File_path::IPv6 &ipv6) { parent->setIPv6Form(ipv6); }
        void operator()(const File_path::SATA &sata) { parent->setSATAForm(sata); }
        void operator()(const File_path::HD &hd) { parent->setHDForm(hd); }
        void operator()(const File_path::File &file) { parent->setFileForm(file); }
        void operator()(const File_path::FirmwareFile &firmware_file) { parent->setFirmwareFileForm(firmware_file); }
        void operator()(const File_path::FirmwareVolume &firmware_volume) { parent->setFirmwareVolumeForm(firmware_volume); }
        void operator()(const File_path::BIOSBootSpecification &bios_boot_specification) { parent->setBIOSBootSpecificationForm(bios_boot_specification); }
        void operator()(const File_path::End &end) { parent->setEndForm(end._subtype); }
        void operator()(const File_path::Unknown &unknown) { parent->setUnknownForm(unknown); }
    };

    std::visit(Visitor(this), *_file_path);

    update();
}

void FilePathDialog::setPCIForm(const File_path::PCI &pci)
{
    ui->options->setCurrentIndex(FormIndex::PCI);
    ui->function_number->setValue(pci.function);
    ui->device_number->setValue(pci.device);
}

void FilePathDialog::setHIDForm(const File_path::HID &hid)
{
    ui->options->setCurrentIndex(FormIndex::HID);
    ui->hid_text->setText(toHex(hid.hid));
    ui->uid_text->setText(toHex(hid.uid));
}

void FilePathDialog::setUSBForm(const File_path::USB &usb)
{
    ui->options->setCurrentIndex(FormIndex::USB);
    ui->parent_port_number->setValue(usb.parent_port_number);
    ui->interface_number->setValue(usb.interface);
}

void FilePathDialog::setVendorForm(const File_path::Vendor &vendor)
{
    ui->options->setCurrentIndex(FormIndex::Vendor);
    VendorTypeIndex index{};
    switch(vendor._type)
    {
    case EFIBoot::File_path::HWVendor::TYPE:
        index = VendorTypeIndex::HW;
        break;

    case EFIBoot::File_path::MSGVendor::TYPE:
        index = VendorTypeIndex::MSG;
        break;

    case EFIBoot::File_path::MEDIAVendor::TYPE:
        index = VendorTypeIndex::MEDIA;
        break;

    default:
        index = VendorTypeIndex::HW;
        break;
    }

    ui->vendor_type_combo->setCurrentIndex(static_cast<int>(index));
    ui->vendor_guid_text->setText(vendor.guid.toString(QUuid::WithoutBraces));
    ui->vendor_data_format_combo->setCurrentIndex(VendorDataFormat::Base64);
    vendor_data_format_combo_index = 0;
    ui->vendor_data_text->setPlainText(vendor.data.toBase64());
}

void FilePathDialog::setMACAddressForm(const File_path::MACAddress &mac_address)
{
    ui->options->setCurrentIndex(FormIndex::MACAddress);
    ui->mac_address_text->setText(mac_address.address);
    ui->if_type_number->setValue(mac_address.if_type);
}

void FilePathDialog::setIPv4Form(const File_path::IPv4 &ipv4)
{
    ui->options->setCurrentIndex(FormIndex::IPv4);
    ui->ipv4_local_ip_text->setText(ipv4.local_ip_address.toString());
    ui->ipv4_local_port_number->setValue(ipv4.local_port);
    ui->ipv4_remote_ip_text->setText(ipv4.remote_ip_address.toString());
    ui->ipv4_remote_port_number->setValue(ipv4.remote_port);
    ui->ipv4_protocol_number->setValue(ipv4.protocol);
    ui->ipv4_static->setChecked(ipv4.static_ip_address);
    ui->ipv4_gateway_ip_text->setText(ipv4.gateway_ip_address.toString());
    ui->ipv4_subnet_mask_text->setText(ipv4.subnet_mask.toString());
}

void FilePathDialog::setIPv6Form(const File_path::IPv6 &ipv6)
{
    ui->options->setCurrentIndex(FormIndex::IPv6);
    ui->ipv6_local_ip_text->setText(ipv6.local_ip_address.toString());
    ui->ipv6_local_port_number->setValue(ipv6.local_port);
    ui->ipv6_remote_ip_text->setText(ipv6.remote_ip_address.toString());
    ui->ipv6_remote_port_number->setValue(ipv6.remote_port);
    ui->ipv6_protocol_number->setValue(ipv6.protocol);
    ui->ipv6_origin_combo->setCurrentIndex(ipv6.ip_address_origin);
    ui->ipv6_gateway_ip_text->setText(ipv6.gateway_ip_address.toString());
    ui->ipv6_prefix_length_number->setValue(ipv6.prefix_length);
}

void FilePathDialog::setSATAForm(const File_path::SATA &sata)
{
    ui->options->setCurrentIndex(FormIndex::SATA);
    ui->hba_port_number->setValue(sata.hba_port);
    ui->port_multiplier_port_number->setValue(sata.port_multiplier_port);
    ui->lun_number->setValue(sata.lun);
}

void FilePathDialog::setHDForm(const File_path::HD &hd)
{
    ui->options->setCurrentIndex(FormIndex::HD);
    for(int index = 0; index < ui->disk_combo->count() - 2; ++index)
    {
        const auto &drive_info = ui->disk_combo->itemData(index).value<DriveInfo>();
        if(static_cast<std::underlying_type_t<DriveInfo::SIGNATURE>>(drive_info.signature_type) == static_cast<std::underlying_type_t<EFIBoot::File_path::HD::SIGNATURE>>(hd.signature_type) && drive_info.partition == hd.partition_number)
        {
            bool found = false;
            switch(static_cast<DriveInfo::SIGNATURE>(drive_info.signature_type))
            {
            case DriveInfo::SIGNATURE::GUID:
                found = drive_info.signature == hd.partition_signature;
                break;
            case DriveInfo::SIGNATURE::MBR:
                found = drive_info.signature.data1 == hd.partition_signature.data1;
                break;
            case DriveInfo::SIGNATURE::NONE:
                break;
            }

            if(found)
            {
                ui->disk_combo->setCurrentIndex(index);
                diskChoiceChanged(index);
                return;
            }
        }
    }

    ui->disk_combo->setCurrentIndex(ui->disk_combo->count() - 1);
    diskChoiceChanged(ui->disk_combo->currentIndex());

    switch(hd.signature_type)
    {
    case EFIBoot::File_path::HD::SIGNATURE::NONE:
        ui->signature_type_combo->setCurrentIndex(0);
        break;

    case EFIBoot::File_path::HD::SIGNATURE::MBR:
        ui->signature_type_combo->setCurrentIndex(1);
        break;

    case EFIBoot::File_path::HD::SIGNATURE::GUID:
        ui->signature_type_combo->setCurrentIndex(2);
        break;
    }

    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    if(hd.signature_type != EFIBoot::File_path::HD::SIGNATURE::NONE)
        ui->signature_text->setText(hd.partition_signature.toString());

    ui->partition_number->setValue(static_cast<int>(hd.partition_number));
    ui->start_text->setText(toHex(hd.partition_start));
    ui->size_text->setText(toHex(hd.partition_size));
}

void FilePathDialog::setFileForm(const File_path::File &file)
{
    ui->options->setCurrentIndex(FormIndex::File);
    ui->filename_text->setText(file.name);
}

void FilePathDialog::setFirmwareFileForm(const File_path::FirmwareFile &firmware_file)
{
    ui->options->setCurrentIndex(FormIndex::FirmwareFile);
    ui->firmware_file_text->setText(firmware_file.name.toString());
}

void FilePathDialog::setFirmwareVolumeForm(const File_path::FirmwareVolume &firmware_volume)
{
    ui->options->setCurrentIndex(FormIndex::FirmwareVolume);
    ui->firmware_volume_text->setText(firmware_volume.name.toString());
}

void FilePathDialog::setBIOSBootSpecificationForm(const File_path::BIOSBootSpecification &bios_boot_specification)
{
    ui->options->setCurrentIndex(FormIndex::BIOSBootSpecification);
    ui->device_type_text->setText(toHex(bios_boot_specification.device_type));
    ui->status_flag_text->setText(toHex(bios_boot_specification.status_flag));
    ui->description_text->setText(bios_boot_specification.description);
}

void FilePathDialog::setEndForm(const uint8_t subtype)
{
    ui->options->setCurrentIndex(FormIndex::End);
    ui->end_subtype_combo->setCurrentIndex(subtype == EFIBoot::File_path::End_instance::SUBTYPE ? 0 : 1);
}

void FilePathDialog::setUnknownForm(const File_path::Unknown &unknown)
{
    ui->options->setCurrentIndex(FormIndex::Unknown_);
    ui->unknown_type_text->setText(toHex(unknown._type));
    ui->unknown_subtype_text->setText(toHex(unknown._subtype));
    ui->unknown_data_format_combo->setCurrentIndex(UnknownDataFormat::Base64);
    unknown_data_format_combo_index = 0;
    ui->unknown_data_text->setPlainText(unknown.data.toBase64());
}

void FilePathDialog::refreshDiskCombo(bool force)
{
    ui->disk_combo->clear();
    const auto drives = DriveInfo::getAll(force);
    for(const auto &drive: drives)
    {
        QVariant item;
        item.setValue(drive);
        ui->disk_combo->addItem(drive.name, item);
    }

    ui->disk_combo->insertSeparator(ui->disk_combo->count());
    ui->disk_combo->addItem("Custom");

    int index = ui->disk_combo->count() - 1;
    ui->disk_combo->setCurrentIndex(index);
    diskChoiceChanged(index);
}

void FilePathDialog::resetDiskCombo()
{
    refreshDiskCombo(true);
}

void FilePathDialog::diskChoiceChanged(int index)
{
    bool disabled = index + 1 != ui->disk_combo->count();
    ui->signature_type_combo->setDisabled(disabled);
    ui->signature_text->setDisabled(disabled);
    ui->partition_number->setDisabled(disabled);
    ui->start_text->setDisabled(disabled);
    ui->size_text->setDisabled(disabled);

    if(index + 1 == ui->disk_combo->count())
        return;

    ui->signature_type_combo->setCurrentIndex(0);
    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();

    const auto &driveinfo = ui->disk_combo->itemData(index).value<DriveInfo>();
    switch(static_cast<DriveInfo::SIGNATURE>(driveinfo.signature_type))
    {
    case DriveInfo::SIGNATURE::NONE:
        ui->signature_type_combo->setCurrentIndex(0);
        break;

    case DriveInfo::SIGNATURE::MBR:
        ui->signature_type_combo->setCurrentIndex(1);
        break;

    case DriveInfo::SIGNATURE::GUID:
        ui->signature_type_combo->setCurrentIndex(2);
        break;
    }

    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    if(driveinfo.signature_type != DriveInfo::SIGNATURE::NONE)
        ui->signature_text->setText(driveinfo.signature.toString());

    ui->partition_number->setValue(static_cast<int>(driveinfo.partition));
    ui->start_text->setText(toHex(driveinfo.start));
    ui->size_text->setText(toHex(driveinfo.size));
}

void FilePathDialog::signatureTypeChoiceChanged(int index)
{
    switch(static_cast<DriveInfo::SIGNATURE>(index))
    {
    case DriveInfo::SIGNATURE::NONE:
        ui->signature_text->setDisabled(true);
        ui->signature_text->setInputMask("");
        ui->signature_text->clear();
        break;

    case DriveInfo::SIGNATURE::MBR:
        ui->signature_text->setDisabled(ui->disk_combo->currentIndex() + 1 != ui->disk_combo->count());
        ui->signature_text->setInputMask("<\\0\\xHHHHHHHH;_");
        ui->signature_text->clear();
        break;

    case DriveInfo::SIGNATURE::GUID:
        ui->signature_text->setDisabled(ui->disk_combo->currentIndex() + 1 != ui->disk_combo->count());
        ui->signature_text->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
        ui->signature_text->clear();
        break;
    }
}

void FilePathDialog::vendorDataFormatChanged(int index)
{
    bool success = false;
    QByteArray input = getVendorData(vendor_data_format_combo_index);
    QString output;
    switch(static_cast<VendorDataFormat>(index))
    {
    case VendorDataFormat::Base64:
        output = input.toBase64();
        success = true;
        break;

    case VendorDataFormat::Utf16:
        if(static_cast<uint>(input.size()) % sizeof(char16_t) == 0)
            success = toUnicode(output, input, "UTF-16");

        break;

    case VendorDataFormat::Utf8:
        success = toUnicode(output, input, "UTF-8");
        break;

    case VendorDataFormat::Hex:
        output = input.toHex();
        success = true;
        break;
    }

    if(output.contains(QChar(0)))
        success = false;

    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change Vendor data format!"));
        ui->vendor_data_format_combo->setCurrentIndex(vendor_data_format_combo_index);
        return;
    }

    ui->vendor_data_text->setPlainText(output);
    vendor_data_format_combo_index = index;
}

void FilePathDialog::unknownDataFormatChanged(int index)
{
    bool success = false;
    QByteArray input = getUnknownData(unknown_data_format_combo_index);
    QString output;
    switch(static_cast<UnknownDataFormat>(index))
    {
    case UnknownDataFormat::Base64:
        output = input.toBase64();
        success = true;
        break;

    case UnknownDataFormat::Utf16:
        if(static_cast<uint>(input.size()) % sizeof(char16_t) == 0)
            success = toUnicode(output, input, "UTF-16");

        break;

    case UnknownDataFormat::Utf8:
        success = toUnicode(output, input, "UTF-8");
        break;

    case UnknownDataFormat::Hex:
        output = input.toHex();
        success = true;
        break;
    }

    if(output.contains(QChar(0)))
        success = false;

    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change Vendor data format!"));
        ui->unknown_data_format_combo->setCurrentIndex(unknown_data_format_combo_index);
        return;
    }

    ui->unknown_data_text->setPlainText(output);
    unknown_data_format_combo_index = index;
}

void FilePathDialog::resetForms()
{
    resetPCIForm();
    resetHIDForm();
    resetVendorForm();
    resetMACAddressForm();
    resetIPv4Form();
    resetIPv6Form();
    resetSATAForm();
    resetHDForm();
    resetFileForm();
    resetFirmwareFileForm();
    resetFirmwareVolumeForm();
    resetEndForm();
    resetUnknownForm();
}

void FilePathDialog::resetPCIForm()
{
    ui->function_number->clear();
    ui->device_number->clear();
}

void FilePathDialog::resetHIDForm()
{
    ui->hid_text->clear();
    ui->uid_text->clear();
}

QByteArray FilePathDialog::getVendorData(int index) const
{
    switch(static_cast<VendorDataFormat>(index))
    {
    case VendorDataFormat::Base64:
        return QByteArray::fromBase64(ui->vendor_data_text->toPlainText().toUtf8());

    case VendorDataFormat::Utf16:
        return fromUnicode(ui->vendor_data_text->toPlainText(), "UTF-16");

    case VendorDataFormat::Utf8:
        return fromUnicode(ui->vendor_data_text->toPlainText(), "UTF-8");

    case VendorDataFormat::Hex:
        return QByteArray::fromHex(ui->vendor_data_text->toPlainText().toUtf8());
    }

    return {};
}

QByteArray FilePathDialog::getUnknownData(int index) const
{
    switch(static_cast<UnknownDataFormat>(index))
    {
    case UnknownDataFormat::Base64:
        return QByteArray::fromBase64(ui->unknown_data_text->toPlainText().toUtf8());

    case UnknownDataFormat::Utf16:
        return fromUnicode(ui->unknown_data_text->toPlainText(), "UTF-16");

    case UnknownDataFormat::Utf8:
        return fromUnicode(ui->unknown_data_text->toPlainText(), "UTF-8");

    case UnknownDataFormat::Hex:
        return QByteArray::fromHex(ui->unknown_data_text->toPlainText().toUtf8());
    }

    return {};
}

void FilePathDialog::resetVendorForm()
{
    ui->vendor_guid_text->clear();
    ui->vendor_type_combo->setCurrentIndex(0);
    ui->vendor_data_format_combo->setCurrentIndex(VendorDataFormat::Base64);
    vendor_data_format_combo_index = 0;
    ui->vendor_data_text->clear();
}

void FilePathDialog::resetMACAddressForm()
{
    ui->mac_address_text->clear();
    ui->if_type_number->clear();
}

void FilePathDialog::resetIPv4Form()
{
    ui->ipv4_local_ip_text->clear();
    ui->ipv4_local_port_number->clear();
    ui->ipv4_remote_ip_text->clear();
    ui->ipv4_remote_port_number->clear();
    ui->ipv4_protocol_number->clear();
    ui->ipv4_static->setChecked(false);
    ui->ipv4_gateway_ip_text->clear();
    ui->ipv4_subnet_mask_text->clear();
}

void FilePathDialog::resetIPv6Form()
{
    ui->ipv6_local_ip_text->clear();
    ui->ipv6_local_port_number->clear();
    ui->ipv6_remote_ip_text->clear();
    ui->ipv6_remote_port_number->clear();
    ui->ipv6_protocol_number->clear();
    ui->ipv6_origin_combo->setCurrentIndex(0);
    ui->ipv6_gateway_ip_text->clear();
    ui->ipv6_prefix_length_number->clear();
}

void FilePathDialog::resetSATAForm()
{
    ui->hba_port_number->clear();
    ui->port_multiplier_port_number->clear();
    ui->lun_number->clear();
}

void FilePathDialog::resetHDForm()
{
    refreshDiskCombo(false);
    ui->signature_type_combo->setCurrentIndex(0);
    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();
}

void FilePathDialog::resetFileForm()
{
    ui->filename_text->clear();
}

void FilePathDialog::resetFirmwareFileForm()
{
    ui->firmware_file_text->clear();
}

void FilePathDialog::resetFirmwareVolumeForm()
{
    ui->firmware_volume_text->clear();
}

void FilePathDialog::resetEndForm()
{
    ui->end_subtype_combo->setCurrentIndex(0);
}

void FilePathDialog::resetUnknownForm()
{
    ui->unknown_type_text->clear();
    ui->unknown_subtype_text->clear();
    ui->unknown_data_format_combo->setCurrentIndex(UnknownDataFormat::Base64);
    unknown_data_format_combo_index = 0;
    ui->unknown_data_text->clear();
}
