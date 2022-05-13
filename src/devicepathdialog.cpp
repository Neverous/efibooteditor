// SPDX-License-Identifier: LGPL-3.0-or-later
#include "devicepathdialog.h"
#include "driveinfo.h"
#include "form/ui_devicepathdialog.h"

#include <QMessageBox>
#include <QTextCodec>

DevicePathDialog::DevicePathDialog(QWidget *parent)
    : QDialog(parent)
    , horizontal_tab_style{}
    , ui{std::make_unique<Ui::DevicePathDialog>()}
    , vendor_data_format_combo_index{}
{
    ui->setupUi(this);
    setDevicePath(nullptr);
    ui->options->tabBar()->setStyle(&horizontal_tab_style);
}

DevicePathDialog::~DevicePathDialog()
{
}

auto DevicePathDialog::toDevicePath() const -> Device_path::ANY
{
    switch(static_cast<FormIndex>(ui->options->currentIndex()))
    {
    case FormIndex::PCI:
    {
        Device_path::PCI pci;
        pci.function = static_cast<quint8>(ui->function_number->value());
        pci.device = static_cast<quint8>(ui->device_number->value());
        return pci;
    }
    case FormIndex::HID:
    {
        Device_path::HID hid;
        hid.hid = ui->hid_text->text().toUInt(nullptr, HEX_BASE);
        hid.uid = ui->uid_text->text().toUInt(nullptr, HEX_BASE);
        return hid;
    }
    case FormIndex::Vendor:
    {
        Device_path::Vendor vendor;
        vendor.guid = QUuid::fromString(ui->vendor_guid_text->text());
        vendor.data = this->getVendorData(ui->vendor_data_format_combo->currentIndex());
        return vendor;
    }
    case FormIndex::MACAddress:
    {
        Device_path::MACAddress mac_address;
        mac_address.address = ui->mac_address_text->text();
        mac_address.if_type = static_cast<quint8>(ui->if_type_number->value());
        return mac_address;
    }
    case FormIndex::IPv4:
    {
        Device_path::IPv4 ipv4;
        ipv4.local_ip_address.setAddress(ui->ipv4_local_ip_text->text());
        ipv4.local_port = static_cast<quint16>(ui->ipv4_local_port_number->value());
        ipv4.remote_ip_address.setAddress(ui->ipv4_remote_ip_text->text());
        ipv4.remote_port = static_cast<quint16>(ui->ipv4_remote_port_number->value());
        ipv4.protocol = static_cast<quint16>(ui->ipv4_protocol_number->value());
        ipv4.static_ip_address = ui->ipv4_static->isChecked();
        ipv4.gateway_ip_address.setAddress(ui->ipv4_gateway_ip_text->text());
        ipv4.subnet_mask.setAddress(ui->ipv4_subnet_mask_text->text());
        return ipv4;
    }
    case FormIndex::IPv6:
    {
        Device_path::IPv6 ipv6;
        ipv6.local_ip_address.setAddress(ui->ipv6_local_ip_text->text());
        ipv6.local_port = static_cast<quint16>(ui->ipv6_local_port_number->value());
        ipv6.remote_ip_address.setAddress(ui->ipv6_remote_ip_text->text());
        ipv6.remote_port = static_cast<quint16>(ui->ipv6_remote_port_number->value());
        ipv6.protocol = static_cast<quint16>(ui->ipv6_protocol_number->value());
        ipv6.ip_address_origin = static_cast<quint8>(ui->ipv6_origin_combo->currentIndex());
        ipv6.gateway_ip_address.setAddress(ui->ipv6_gateway_ip_text->text());
        ipv6.prefix_length = static_cast<quint8>(ui->ipv6_prefix_length_number->value());
        return ipv6;
    }
    case FormIndex::SATA:
    {
        Device_path::SATA sata;
        sata.hba_port = static_cast<quint16>(ui->hba_port_number->value());
        sata.port_multiplier_port = static_cast<quint16>(ui->port_multiplier_port_number->value());
        sata.lun = static_cast<quint16>(ui->lun_number->value());
        return sata;
    }
    case FormIndex::HD:
    {
        Device_path::HD hd;
        hd.signature_type = static_cast<quint8>(ui->signature_type_combo->currentIndex());
        hd.partition_format = hd.signature_type ? hd.signature_type : static_cast<quint8>(EFIBoot::Device_path::SIGNATURE::MBR);
        hd.partition_number = static_cast<quint32>(ui->partition_number->value());
        switch(static_cast<EFIBoot::Device_path::SIGNATURE>(hd.signature_type))
        {
        case EFIBoot::Device_path::SIGNATURE::GUID:
            hd.partition_signature = QUuid::fromString(ui->signature_text->text());
            break;

        case EFIBoot::Device_path::SIGNATURE::MBR:
            hd.partition_signature = QUuid{ui->signature_text->text().toUInt(nullptr, HEX_BASE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            break;

        case EFIBoot::Device_path::SIGNATURE::NONE:
            hd.partition_signature = {};
            break;
        }

        hd.partition_start = ui->start_text->text().toULongLong(nullptr, HEX_BASE);
        hd.partition_size = ui->size_text->text().toULongLong(nullptr, HEX_BASE);
        return hd;
    }

    case FormIndex::File:
    {
        Device_path::File file;
        file.name = ui->filename_text->text();
        return file;
    }

    case FormIndex::FirmwareFile:
    {
        Device_path::FirmwareFile firmware_file;
        firmware_file.name = QUuid::fromString(ui->firmware_file_text->text());
        return firmware_file;
    }

    case FormIndex::FirmwareVolume:
    {
        Device_path::FirmwareVolume firmware_volume;
        firmware_volume.name = QUuid::fromString(ui->firmware_volume_text->text());
        return firmware_volume;
    }
    }

    return {};
}

void DevicePathDialog::setDevicePath(const Device_path::ANY *_device_path)
{
    resetForms();
    if(!_device_path)
    {
        update();
        return;
    }

    std::visit(
        overloaded{
            // clang-format off
            [&](const Device_path::PCI &pci) { setPCIForm(pci); },
            [&](const Device_path::HID &hid) { setHIDForm(hid); },
            [&](const Device_path::Vendor &vendor) { setVendorForm(vendor); },
            [&](const Device_path::MACAddress &mac_address) { setMACAddressForm(mac_address); },
            [&](const Device_path::IPv4 &ipv4) { setIPv4Form(ipv4); },
            [&](const Device_path::IPv6 &ipv6) { setIPv6Form(ipv6); },
            [&](const Device_path::SATA &sata) { setSATAForm(sata); },
            [&](const Device_path::HD &hd) { setHDForm(hd); },
            [&](const Device_path::File &file) { setFileForm(file); },
            [&](const Device_path::FirmwareFile &firmware_file) { setFirmwareFileForm(firmware_file); },
            [&](const Device_path::FirmwareVolume &firmware_volume) { setFirmwareVolumeForm(firmware_volume); },
            // clang-format on
        },
        *_device_path);

    update();
}

void DevicePathDialog::setPCIForm(const Device_path::PCI &pci)
{
    ui->options->setCurrentIndex(FormIndex::PCI);
    ui->function_number->setValue(pci.function);
    ui->device_number->setValue(pci.device);
}

void DevicePathDialog::setHIDForm(const Device_path::HID &hid)
{
    ui->options->setCurrentIndex(FormIndex::HID);
    ui->hid_text->setText(QString::number(hid.hid, HEX_BASE));
    ui->uid_text->setText(QString::number(hid.uid, HEX_BASE));
}

void DevicePathDialog::setVendorForm(const Device_path::Vendor &vendor)
{
    ui->options->setCurrentIndex(FormIndex::Vendor);
    ui->vendor_guid_text->setText(vendor.guid.toString(QUuid::WithoutBraces));
    ui->vendor_data_format_combo->setCurrentIndex(0); // BASE64
    ui->vendor_data_text->setPlainText(vendor.data.toBase64());
}

void DevicePathDialog::setMACAddressForm(const Device_path::MACAddress &mac_address)
{
    ui->options->setCurrentIndex(FormIndex::MACAddress);
    ui->mac_address_text->setText(mac_address.address);
    ui->if_type_number->setValue(mac_address.if_type);
}

void DevicePathDialog::setIPv4Form(const Device_path::IPv4 &ipv4)
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

void DevicePathDialog::setIPv6Form(const Device_path::IPv6 &ipv6)
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

void DevicePathDialog::setSATAForm(const Device_path::SATA &sata)
{
    ui->options->setCurrentIndex(FormIndex::SATA);
    ui->hba_port_number->setValue(sata.hba_port);
    ui->port_multiplier_port_number->setValue(sata.port_multiplier_port);
    ui->lun_number->setValue(sata.lun);
}

void DevicePathDialog::setHDForm(const Device_path::HD &hd)
{
    ui->options->setCurrentIndex(FormIndex::HD);
    for(int index = 0; index < ui->disk_combo->count() - 2; ++index)
    {
        const auto &drive_info = ui->disk_combo->itemData(index).value<DriveInfo>();
        if(static_cast<quint8>(drive_info.signature_type) == hd.signature_type && drive_info.partition == hd.partition_number)
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

    switch(static_cast<EFIBoot::Device_path::SIGNATURE>(hd.signature_type))
    {
    case EFIBoot::Device_path::SIGNATURE::NONE:
        ui->signature_type_combo->setCurrentIndex(0);
        break;

    case EFIBoot::Device_path::SIGNATURE::MBR:
        ui->signature_type_combo->setCurrentIndex(1);
        break;

    case EFIBoot::Device_path::SIGNATURE::GUID:
        ui->signature_type_combo->setCurrentIndex(2);
        break;
    }

    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    if(hd.signature_type != EFIBoot::Device_path::SIGNATURE::NONE)
        ui->signature_text->setText(hd.partition_signature.toString());

    ui->partition_number->setValue(static_cast<int>(hd.partition_number));
    ui->start_text->setText(QString::number(hd.partition_start, HEX_BASE));
    ui->size_text->setText(QString::number(hd.partition_size, HEX_BASE));
}

void DevicePathDialog::setFileForm(const Device_path::File &file)
{
    ui->options->setCurrentIndex(FormIndex::File);
    ui->filename_text->setText(file.name);
}

void DevicePathDialog::setFirmwareFileForm(const Device_path::FirmwareFile &firmware_file)
{
    ui->options->setCurrentIndex(FormIndex::FirmwareFile);
    ui->firmware_file_text->setText(firmware_file.name.toString());
}

void DevicePathDialog::setFirmwareVolumeForm(const Device_path::FirmwareVolume &firmware_volume)
{
    ui->options->setCurrentIndex(FormIndex::FirmwareVolume);
    ui->firmware_volume_text->setText(firmware_volume.name.toString());
}

void DevicePathDialog::refreshDiskCombo(bool force)
{
    ui->disk_combo->clear();
    const auto drives = DriveInfo::get_all(force);
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

void DevicePathDialog::resetDiskCombo()
{
    refreshDiskCombo(true);
}

void DevicePathDialog::diskChoiceChanged(int index)
{
    bool disabled = index + 1 != ui->disk_combo->count();
    ui->signature_type_combo->setDisabled(disabled);
    ui->signature_text->setDisabled(disabled);
    ui->partition_number->setDisabled(disabled);
    ui->start_text->setDisabled(disabled);
    ui->size_text->setDisabled(disabled);

    ui->signature_type_combo->setCurrentIndex(0);
    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();

    if(index + 1 == ui->disk_combo->count())
        return;

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
    ui->start_text->setText(QString::number(driveinfo.start, HEX_BASE));
    ui->size_text->setText(QString::number(driveinfo.size, HEX_BASE));
}

void DevicePathDialog::signatureTypeChoiceChanged(int index)
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

void DevicePathDialog::vendorDataFormatChanged(int index)
{
    QTextCodec *codec = nullptr;
    QTextCodec::ConverterState state;
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
        {
            codec = QTextCodec::codecForName("UTF-16");
            output = codec->toUnicode(input.constData(), static_cast<int>(input.size()), &state);
            success = state.invalidChars == 0;
        }
        break;

    case VendorDataFormat::Utf8:
        codec = QTextCodec::codecForName("UTF-8");
        output = codec->toUnicode(input.constData(), static_cast<int>(input.size()), &state);
        success = state.invalidChars == 0;
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

void DevicePathDialog::resetForms()
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
}

void DevicePathDialog::resetPCIForm()
{
    ui->function_number->clear();
    ui->device_number->clear();
}

void DevicePathDialog::resetHIDForm()
{
    ui->hid_text->clear();
    ui->uid_text->clear();
}

QByteArray DevicePathDialog::getVendorData(int index) const
{
    std::unique_ptr<QTextEncoder> encoder = nullptr;
    switch(static_cast<VendorDataFormat>(index))
    {
    case VendorDataFormat::Base64:
        return QByteArray::fromBase64(ui->vendor_data_text->toPlainText().toUtf8());
        break;

    case VendorDataFormat::Utf16:
        encoder.reset(QTextCodec::codecForName("UTF-16")->makeEncoder(QTextCodec::IgnoreHeader));
        return encoder->fromUnicode(ui->vendor_data_text->toPlainText());
        break;

    case VendorDataFormat::Utf8:
        encoder.reset(QTextCodec::codecForName("UTF-8")->makeEncoder(QTextCodec::IgnoreHeader));
        return encoder->fromUnicode(ui->vendor_data_text->toPlainText());
        break;

    case VendorDataFormat::Hex:
        return QByteArray::fromHex(ui->vendor_data_text->toPlainText().toUtf8());
        break;
    }

    return {};
}

void DevicePathDialog::resetVendorForm()
{
    ui->vendor_guid_text->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
    ui->vendor_guid_text->clear();
    ui->vendor_data_format_combo->setCurrentIndex(0);
    vendor_data_format_combo_index = 0;
    ui->vendor_data_text->clear();
}

void DevicePathDialog::resetMACAddressForm()
{
    ui->mac_address_text->setInputMask("<hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh:hh;0");
    ui->mac_address_text->clear();
    ui->if_type_number->clear();
}

void DevicePathDialog::resetIPv4Form()
{
    ui->ipv4_local_ip_text->setInputMask("000.000.000.000");
    ui->ipv4_local_ip_text->clear();
    ui->ipv4_local_port_number->clear();
    ui->ipv4_remote_ip_text->setInputMask("000.000.000.000");
    ui->ipv4_remote_ip_text->clear();
    ui->ipv4_remote_port_number->clear();
    ui->ipv4_protocol_number->clear();
    ui->ipv4_static->setChecked(false);
    ui->ipv4_gateway_ip_text->setInputMask("000.000.000.000");
    ui->ipv4_gateway_ip_text->clear();
    ui->ipv4_subnet_mask_text->setInputMask("000.000.000.000");
    ui->ipv4_subnet_mask_text->clear();
}

void DevicePathDialog::resetIPv6Form()
{
    ui->ipv6_local_ip_text->setInputMask("<hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh");
    ui->ipv6_local_ip_text->clear();
    ui->ipv6_local_port_number->clear();
    ui->ipv6_remote_ip_text->setInputMask("<hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh");
    ui->ipv6_remote_ip_text->clear();
    ui->ipv6_remote_port_number->clear();
    ui->ipv6_protocol_number->clear();
    ui->ipv6_origin_combo->setCurrentIndex(0);
    ui->ipv6_gateway_ip_text->setInputMask("<hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh");
    ui->ipv6_gateway_ip_text->clear();
    ui->ipv6_prefix_length_number->clear();
}

void DevicePathDialog::resetSATAForm()
{
    ui->hba_port_number->clear();
    ui->port_multiplier_port_number->clear();
    ui->lun_number->clear();
}

void DevicePathDialog::resetHDForm()
{
    refreshDiskCombo(false);
    ui->signature_type_combo->setCurrentIndex(0);
    signatureTypeChoiceChanged(ui->signature_type_combo->currentIndex());
    ui->partition_number->clear();
    ui->start_text->clear();
    ui->size_text->clear();
}

void DevicePathDialog::resetFileForm()
{
    ui->filename_text->clear();
}

void DevicePathDialog::resetFirmwareFileForm()
{
    ui->firmware_file_text->clear();
}

void DevicePathDialog::resetFirmwareVolumeForm()
{
    ui->firmware_volume_text->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
    ui->firmware_volume_text->clear();
}
