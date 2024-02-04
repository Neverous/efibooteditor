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

    ui->hd_disk_refresh->setDisabled(readonly);
    ui->options->tabBar()->setDisabled(readonly);
    ui->adr_additional_adr_format->setDisabled(false);
    ui->dns_data_format->setDisabled(false);
    ui->rest_service_data_format->setDisabled(false);
    ui->unknown_data_format->setDisabled(false);
    ui->vendor_data_format->setDisabled(false);
}

auto FilePathDialog::toFilePath() const -> FilePath::ANY
{
    switch(static_cast<FormIndex>(ui->options->currentIndex()))
    {
    // Hardware
    case FormIndex::PCI:
    {
        FilePath::Pci value{};
        value.function = static_cast<decltype(value.function)>(ui->pci_function->value());
        value.device = static_cast<decltype(value.device)>(ui->pci_device->value());
        return value;
    }
    case FormIndex::PCCARD:
    {
        FilePath::Pccard value{};
        value.function_number = static_cast<decltype(value.function_number)>(ui->pccard_function_number->value());
        return value;
    }
    case FormIndex::MEMORY_MAPPED:
    {
        FilePath::MemoryMapped value{};
        value.memory_type = static_cast<decltype(value.memory_type)>(ui->memory_mapped_memory_type->currentIndex());
        value.start_address = ui->memory_mapped_start_address->text().toULongLong(nullptr, HEX_BASE);
        value.end_address = ui->memory_mapped_end_address->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::CONTROLLER:
    {
        FilePath::Controller value{};
        value.controller_number = static_cast<decltype(value.controller_number)>(ui->controller_controller_number->value());
        return value;
    }
    case FormIndex::BMC:
    {
        FilePath::Bmc value{};
        value.interface_type = static_cast<decltype(value.interface_type)>(ui->bmc_interface_type->currentIndex());
        value.base_address = ui->bmc_base_address->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    // ACPI
    case FormIndex::ACPI:
    {
        FilePath::Acpi value{};
        value.hid = ui->acpi_hid->text().toUInt(nullptr, HEX_BASE);
        value.uid = ui->acpi_uid->text().toUInt(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::EXPANDED:
    {
        FilePath::Expanded value{};
        value.hid = ui->expanded_hid->text().toUInt(nullptr, HEX_BASE);
        value.uid = ui->expanded_uid->text().toUInt(nullptr, HEX_BASE);
        value.cid = ui->expanded_cid->text().toUInt(nullptr, HEX_BASE);
        value.hidstr = ui->expanded_hidstr->text();
        value.uidstr = ui->expanded_uidstr->text();
        value.cidstr = ui->expanded_cidstr->text();
        return value;
    }
    case FormIndex::ADR:
    {
        FilePath::Adr value{};
        value.adr = ui->adr_adr->text().toUInt(nullptr, HEX_BASE);
        value.additional_adr = getData(*ui->adr_additional_adr, ui->adr_additional_adr_format->currentIndex());
        return value;
    }
    case FormIndex::NVDIMM:
    {
        FilePath::Nvdimm value{};
        value.nfit_device_handle = ui->nvdimm_nfit_device_handle->text().toUInt(nullptr, HEX_BASE);
        return value;
    }
    // Messaging
    case FormIndex::ATAPI:
    {
        FilePath::Atapi value{};
        value.primary = ui->atapi_primary->isChecked();
        value.slave = ui->atapi_slave->isChecked();
        value.lun = static_cast<decltype(value.lun)>(ui->atapi_lun->value());
        return value;
    }
    case FormIndex::SCSI:
    {
        FilePath::Scsi value{};
        value.pun = static_cast<decltype(value.pun)>(ui->scsi_pun->value());
        value.lun = static_cast<decltype(value.lun)>(ui->scsi_lun->value());
        return value;
    }
    case FormIndex::FIBRE_CHANNEL:
    {
        FilePath::FibreChannel value{};
        value.reserved = ui->fibre_channel_reserved->text().toUInt(nullptr, HEX_BASE);
        value.world_wide_name = ui->fibre_channel_world_wide_name->text().toULongLong(nullptr, HEX_BASE);
        value.lun = ui->fibre_channel_lun->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::FIREWIRE:
    {
        FilePath::Firewire value{};
        value.reserved = ui->firewire_reserved->text().toUInt(nullptr, HEX_BASE);
        value.guid = ui->firewire_guid->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::USB:
    {
        FilePath::Usb value{};
        value.parent_port_number = static_cast<decltype(value.parent_port_number)>(ui->usb_parent_port_number->value());
        value.interface_number = static_cast<decltype(value.interface_number)>(ui->usb_interface_number->value());
        return value;
    }
    case FormIndex::I2O:
    {
        FilePath::I2o value{};
        value.tid = static_cast<decltype(value.tid)>(ui->i2o_tid->value());
        return value;
    }
    case FormIndex::INFINIBAND:
    {
        FilePath::Infiniband value{};
        value.resource_flags = ui->infiniband_resource_flags->text().toUInt(nullptr, HEX_BASE);
        value.port_gid = QUuid::fromString(ui->infiniband_port_gid->text());
        value.ioc_guid_service_id = ui->infiniband_ioc_guid_service_id->text().toULongLong(nullptr, HEX_BASE);
        value.target_port_id = ui->infiniband_target_port_id->text().toULongLong(nullptr, HEX_BASE);
        value.device_id = ui->infiniband_device_id->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::MAC_ADDRESS:
    {
        FilePath::MacAddress value{};
        value.address = ui->mac_address_address->text();
        value.if_type = static_cast<decltype(value.if_type)>(ui->mac_address_if_type->value());
        return value;
    }
    case FormIndex::IPV4:
    {
        FilePath::Ipv4 value{};
        value.local_ip_address.setAddress(ui->ipv4_local_ip_address->text());
        value.remote_ip_address.setAddress(ui->ipv4_remote_ip_address->text());
        value.local_port = static_cast<decltype(value.local_port)>(ui->ipv4_local_port->value());
        value.remote_port = static_cast<decltype(value.remote_port)>(ui->ipv4_remote_port->value());
        value.protocol = static_cast<decltype(value.protocol)>(ui->ipv4_protocol->value());
        value.static_ip_address = ui->ipv4_static_ip_address->isChecked();
        value.gateway_ip_address.setAddress(ui->ipv4_gateway_ip_address->text());
        value.subnet_mask.setAddress(ui->ipv4_subnet_mask->text());
        return value;
    }
    case FormIndex::IPV6:
    {
        FilePath::Ipv6 value{};
        value.local_ip_address.setAddress(ui->ipv6_local_ip_address->text());
        value.remote_ip_address.setAddress(ui->ipv6_remote_ip_address->text());
        value.local_port = static_cast<decltype(value.local_port)>(ui->ipv6_local_port->value());
        value.remote_port = static_cast<decltype(value.remote_port)>(ui->ipv6_remote_port->value());
        value.protocol = static_cast<decltype(value.protocol)>(ui->ipv6_protocol->value());
        value.ip_address_origin = static_cast<decltype(value.ip_address_origin)>(ui->ipv6_ip_address_origin->currentIndex());
        value.prefix_length = static_cast<decltype(value.prefix_length)>(ui->ipv6_prefix_length->value());
        value.gateway_ip_address.setAddress(ui->ipv6_gateway_ip_address->text());
        return value;
    }
    case FormIndex::UART:
    {
        FilePath::Uart value{};
        value.reserved = ui->uart_reserved->text().toUInt(nullptr, HEX_BASE);
        value.baud_rate = ui->uart_baud_rate->text().toULongLong(nullptr, HEX_BASE);
        value.data_bits = static_cast<decltype(value.data_bits)>(ui->uart_data_bits->value());
        value.parity = static_cast<decltype(value.parity)>(ui->uart_parity->currentIndex());
        value.stop_bits = static_cast<decltype(value.stop_bits)>(ui->uart_stop_bits->currentIndex());
        return value;
    }
    case FormIndex::USB_CLASS:
    {
        FilePath::UsbClass value{};
        value.vendor_id = ui->usb_class_vendor_id->text().toUShort(nullptr, HEX_BASE);
        value.product_id = ui->usb_class_product_id->text().toUShort(nullptr, HEX_BASE);
        value.device_class = static_cast<decltype(value.device_class)>(ui->usb_class_device_class->text().toUShort(nullptr, HEX_BASE));
        value.device_subclass = static_cast<decltype(value.device_subclass)>(ui->usb_class_device_subclass->text().toUShort(nullptr, HEX_BASE));
        value.device_protocol = static_cast<decltype(value.device_protocol)>(ui->usb_class_device_protocol->text().toUShort(nullptr, HEX_BASE));
        return value;
    }
    case FormIndex::USB_WWID:
    {
        FilePath::UsbWwid value{};
        value.interface_number = static_cast<decltype(value.interface_number)>(ui->usb_wwid_interface_number->value());
        value.device_vendor_id = ui->usb_wwid_device_vendor_id->text().toUShort(nullptr, HEX_BASE);
        value.device_product_id = ui->usb_wwid_device_product_id->text().toUShort(nullptr, HEX_BASE);
        value.serial_number = ui->usb_wwid_serial_number->text();
        return value;
    }
    case FormIndex::DEVICE_LOGICAL_UNIT:
    {
        FilePath::DeviceLogicalUnit value{};
        value.lun = static_cast<decltype(value.lun)>(ui->device_logical_unit_lun->value());
        return value;
    }
    case FormIndex::SATA:
    {
        FilePath::Sata value{};
        value.hba_port_number = static_cast<decltype(value.hba_port_number)>(ui->sata_hba_port_number->value());
        value.port_multiplier_port_number = static_cast<decltype(value.port_multiplier_port_number)>(ui->sata_port_multiplier_port_number->value());
        value.lun = static_cast<decltype(value.lun)>(ui->sata_lun->value());
        return value;
    }
    case FormIndex::ISCSI:
    {
        FilePath::Iscsi value{};
        value.protocol = static_cast<decltype(value.protocol)>(ui->iscsi_protocol->value());
        value.options = ui->iscsi_options->text().toUShort(nullptr, HEX_BASE);
        value.lun = ui->iscsi_lun->text().toULongLong(nullptr, HEX_BASE);
        value.target_portal_group = static_cast<decltype(value.target_portal_group)>(ui->iscsi_target_portal_group->value());
        value.target_name = ui->iscsi_target_name->text();
        return value;
    }
    case FormIndex::VLAN:
    {
        FilePath::Vlan value{};
        value.vlan_id = static_cast<decltype(value.vlan_id)>(ui->vlan_vlan_id->value());
        return value;
    }
    case FormIndex::FIBRE_CHANNEL_EX:
    {
        FilePath::FibreChannelEx value{};
        value.reserved = ui->fibre_channel_ex_reserved->text().toUInt(nullptr, HEX_BASE);
        value.world_wide_name = ui->fibre_channel_ex_world_wide_name->text().toULongLong(nullptr, HEX_BASE);
        value.lun = ui->fibre_channel_ex_lun->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::SAS_EXTENDED_MESSAGING:
    {
        FilePath::SasExtendedMessaging value{};
        value.sas_address = ui->sas_extended_messaging_sas_address->text().toULongLong(nullptr, HEX_BASE);
        value.lun = ui->sas_extended_messaging_lun->text().toULongLong(nullptr, HEX_BASE);
        value.device_and_topology_info = ui->sas_extended_messaging_device_and_topology_info->text().toUShort(nullptr, HEX_BASE);
        value.relative_target_port = static_cast<decltype(value.relative_target_port)>(ui->sas_extended_messaging_relative_target_port->value());
        return value;
    }
    case FormIndex::NVM_EXPRESS_NS:
    {
        FilePath::NvmExpressNs value{};
        value.namespace_identifier = ui->nvm_express_ns_namespace_identifier->text().toUInt(nullptr, HEX_BASE);
        value.ieee_extended_unique_identifier = ui->nvm_express_ns_ieee_extended_unique_identifier->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::URI:
    {
        FilePath::Uri value{};
        value.uri = QUrl::fromUserInput(ui->uri_uri->text());
        return value;
    }
    case FormIndex::UFS:
    {
        FilePath::Ufs value{};
        value.pun = static_cast<decltype(value.pun)>(ui->ufs_pun->text().toUShort(nullptr, HEX_BASE));
        value.lun = static_cast<decltype(value.lun)>(ui->ufs_lun->text().toUShort(nullptr, HEX_BASE));
        return value;
    }
    case FormIndex::SD:
    {
        FilePath::Sd value{};
        value.slot_number = static_cast<decltype(value.slot_number)>(ui->sd_slot_number->value());
        return value;
    }
    case FormIndex::BLUETOOTH:
    {
        FilePath::Bluetooth value{};
        value.device_address = ui->bluetooth_device_address->text();
        return value;
    }
    case FormIndex::WI_FI:
    {
        FilePath::WiFi value{};
        value.ssid = ui->wi_fi_ssid->text();
        return value;
    }
    case FormIndex::EMMC:
    {
        FilePath::Emmc value{};
        value.slot_number = static_cast<decltype(value.slot_number)>(ui->emmc_slot_number->value());
        return value;
    }
    case FormIndex::BLUETOOTHLE:
    {
        FilePath::Bluetoothle value{};
        value.device_address = ui->bluetoothle_device_address->text();
        value.address_type = static_cast<decltype(value.address_type)>(ui->bluetoothle_address_type->currentIndex());
        return value;
    }
    case FormIndex::DNS:
    {
        FilePath::Dns value{};
        value.ipv6 = ui->dns_ipv6->isChecked();
        value.data = getData(*ui->dns_data, ui->dns_data_format->currentIndex());
        return value;
    }
    case FormIndex::NVDIMM_NS:
    {
        FilePath::NvdimmNs value{};
        value.uuid = QUuid::fromString(ui->nvdimm_ns_uuid->text());
        return value;
    }
    case FormIndex::REST_SERVICE:
    {
        FilePath::RestService value{};
        value.rest_service = static_cast<decltype(value.rest_service)>(ui->rest_service_rest_service->currentIndex());
        value.access_mode = static_cast<decltype(value.access_mode)>(ui->rest_service_access_mode->currentIndex());
        value.guid = QUuid::fromString(ui->rest_service_guid->text());
        value.data = getData(*ui->rest_service_data, ui->rest_service_data_format->currentIndex());
        return value;
    }
    case FormIndex::NVME_OF_NS:
    {
        FilePath::NvmeOfNs value{};
        value.nidt = static_cast<decltype(value.nidt)>(ui->nvme_of_ns_nidt->value());
        value.nid = QUuid::fromString(ui->nvme_of_ns_nid->text());
        value.subsystem_nqn = ui->nvme_of_ns_subsystem_nqn->text();
        return value;
    }
    // Media
    case FormIndex::HD:
    {
        FilePath::Hd value{};
        value.partition_number = static_cast<decltype(value.partition_number)>(ui->hd_partition_number->value());
        value.partition_start = ui->hd_partition_start->text().toULongLong(nullptr, HEX_BASE);
        value.partition_size = ui->hd_partition_size->text().toULongLong(nullptr, HEX_BASE);
        value.signature_type = static_cast<decltype(value.signature_type)>(ui->hd_signature_type->currentIndex());
        value.partition_format = static_cast<decltype(value.partition_format)>(value.signature_type != EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::NONE ? value.signature_type : EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::MBR);

        switch(value.signature_type)
        {
        case EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::GUID:
            value.partition_signature = QUuid::fromString(ui->hd_partition_signature->text());
            break;

        case EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::MBR:
            value.partition_signature = QUuid{ui->hd_partition_signature->text().toUInt(nullptr, HEX_BASE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            break;

        case EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::NONE:
            value.partition_signature = QUuid{};
            break;
        }

        return value;
    }
    case FormIndex::CD_ROM:
    {
        FilePath::CdRom value{};
        value.boot_entry = static_cast<decltype(value.boot_entry)>(ui->cd_rom_boot_entry->value());
        value.partition_start = ui->cd_rom_partition_start->text().toULongLong(nullptr, HEX_BASE);
        value.partition_size = ui->cd_rom_partition_size->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::FILE_PATH:
    {
        FilePath::FilePath value{};
        value.path_name = ui->file_path_path_name->text();
        return value;
    }
    case FormIndex::PROTOCOL:
    {
        FilePath::Protocol value{};
        value.guid = QUuid::fromString(ui->protocol_guid->text());
        return value;
    }
    case FormIndex::FIRMWARE_FILE:
    {
        FilePath::FirmwareFile value{};
        value.name = QUuid::fromString(ui->firmware_file_name->text());
        return value;
    }
    case FormIndex::FIRMWARE_VOLUME:
    {
        FilePath::FirmwareVolume value{};
        value.name = QUuid::fromString(ui->firmware_volume_name->text());
        return value;
    }
    case FormIndex::RELATIVE_OFFSET_RANGE:
    {
        FilePath::RelativeOffsetRange value{};
        value.reserved = ui->relative_offset_range_reserved->text().toUInt(nullptr, HEX_BASE);
        value.starting_offset = ui->relative_offset_range_starting_offset->text().toULongLong(nullptr, HEX_BASE);
        value.ending_offset = ui->relative_offset_range_ending_offset->text().toULongLong(nullptr, HEX_BASE);
        return value;
    }
    case FormIndex::RAM_DISK:
    {
        FilePath::RamDisk value{};
        value.starting_address = ui->ram_disk_starting_address->text().toULongLong(nullptr, HEX_BASE);
        value.ending_address = ui->ram_disk_ending_address->text().toULongLong(nullptr, HEX_BASE);
        value.guid = QUuid::fromString(ui->ram_disk_guid->text());
        value.disk_instance = static_cast<decltype(value.disk_instance)>(ui->ram_disk_disk_instance->value());
        return value;
    }
    // BIOS
    case FormIndex::BOOT_SPECIFICATION:
    {
        FilePath::BootSpecification value{};
        value.device_type = ui->boot_specification_device_type->text().toUShort(nullptr, HEX_BASE);
        value.status_flag = ui->boot_specification_status_flag->text().toUShort(nullptr, HEX_BASE);
        value.description = ui->boot_specification_description->text();
        return value;
    }
    // Rest
    case FormIndex::VENDOR:
    {
        FilePath::Vendor vendor;
        switch(static_cast<VendorTypeIndex>(ui->vendor_type->currentIndex()))
        {
        case VendorTypeIndex::HW:
            vendor._type = EFIBoot::File_path::HW::Vendor::TYPE;
            break;

        case VendorTypeIndex::MSG:
            vendor._type = EFIBoot::File_path::MSG::Vendor::TYPE;
            break;

        case VendorTypeIndex::MEDIA:
            vendor._type = EFIBoot::File_path::MEDIA::Vendor::TYPE;
            break;
        }

        vendor.guid = QUuid::fromString(ui->vendor_guid->text());
        vendor.data = getData(*ui->vendor_data, ui->vendor_data_format->currentIndex());
        return vendor;
    }
    case FormIndex::END:
    {
        FilePath::End end;
        if(ui->end_subtype->currentIndex() == 0)
            end._subtype = EFIBoot::File_path::END::Instance::SUBTYPE;

        else
            end._subtype = EFIBoot::File_path::END::Entire::SUBTYPE;

        return end;
    }
    case FormIndex::UNKNOWN:
    {
        FilePath::Unknown unknown;
        unknown._type = static_cast<uint8_t>(ui->unknown_type->text().toUShort(nullptr, HEX_BASE));
        unknown._subtype = static_cast<uint8_t>(ui->unknown_subtype->text().toUShort(nullptr, HEX_BASE));
        unknown.data = getData(*ui->unknown_data, ui->unknown_data_format->currentIndex());
        return unknown;
    }
    }

    return {};
}

struct FilePathVisitor
{
    FilePathDialog *parent;
    explicit FilePathVisitor(FilePathDialog *parent_)
        : parent{parent_}
    {
    }

    void operator()(const FilePath::Pci &pci) { parent->setPciForm(pci); }
    void operator()(const FilePath::Pccard &pccard) { parent->setPccardForm(pccard); }
    void operator()(const FilePath::MemoryMapped &memory_mapped) { parent->setMemoryMappedForm(memory_mapped); }
    void operator()(const FilePath::Controller &controller) { parent->setControllerForm(controller); }
    void operator()(const FilePath::Bmc &bmc) { parent->setBmcForm(bmc); }
    void operator()(const FilePath::Acpi &acpi) { parent->setAcpiForm(acpi); }
    void operator()(const FilePath::Expanded &expanded) { parent->setExpandedForm(expanded); }
    void operator()(const FilePath::Adr &adr) { parent->setAdrForm(adr); }
    void operator()(const FilePath::Nvdimm &nvdimm) { parent->setNvdimmForm(nvdimm); }
    void operator()(const FilePath::Atapi &atapi) { parent->setAtapiForm(atapi); }
    void operator()(const FilePath::Scsi &scsi) { parent->setScsiForm(scsi); }
    void operator()(const FilePath::FibreChannel &fibre_channel) { parent->setFibreChannelForm(fibre_channel); }
    void operator()(const FilePath::Firewire &firewire) { parent->setFirewireForm(firewire); }
    void operator()(const FilePath::Usb &usb) { parent->setUsbForm(usb); }
    void operator()(const FilePath::I2o &i2o) { parent->setI2oForm(i2o); }
    void operator()(const FilePath::Infiniband &infiniband) { parent->setInfinibandForm(infiniband); }
    void operator()(const FilePath::MacAddress &mac_address) { parent->setMacAddressForm(mac_address); }
    void operator()(const FilePath::Ipv4 &ipv4) { parent->setIpv4Form(ipv4); }
    void operator()(const FilePath::Ipv6 &ipv6) { parent->setIpv6Form(ipv6); }
    void operator()(const FilePath::Uart &uart) { parent->setUartForm(uart); }
    void operator()(const FilePath::UsbClass &usb_class) { parent->setUsbClassForm(usb_class); }
    void operator()(const FilePath::UsbWwid &usb_wwid) { parent->setUsbWwidForm(usb_wwid); }
    void operator()(const FilePath::DeviceLogicalUnit &device_logical_unit) { parent->setDeviceLogicalUnitForm(device_logical_unit); }
    void operator()(const FilePath::Sata &sata) { parent->setSataForm(sata); }
    void operator()(const FilePath::Iscsi &iscsi) { parent->setIscsiForm(iscsi); }
    void operator()(const FilePath::Vlan &vlan) { parent->setVlanForm(vlan); }
    void operator()(const FilePath::FibreChannelEx &fibre_channel_ex) { parent->setFibreChannelExForm(fibre_channel_ex); }
    void operator()(const FilePath::SasExtendedMessaging &sas_extended_messaging) { parent->setSasExtendedMessagingForm(sas_extended_messaging); }
    void operator()(const FilePath::NvmExpressNs &nvm_express_ns) { parent->setNvmExpressNsForm(nvm_express_ns); }
    void operator()(const FilePath::Uri &uri) { parent->setUriForm(uri); }
    void operator()(const FilePath::Ufs &ufs) { parent->setUfsForm(ufs); }
    void operator()(const FilePath::Sd &sd) { parent->setSdForm(sd); }
    void operator()(const FilePath::Bluetooth &bluetooth) { parent->setBluetoothForm(bluetooth); }
    void operator()(const FilePath::WiFi &wi_fi) { parent->setWiFiForm(wi_fi); }
    void operator()(const FilePath::Emmc &emmc) { parent->setEmmcForm(emmc); }
    void operator()(const FilePath::Bluetoothle &bluetoothle) { parent->setBluetoothleForm(bluetoothle); }
    void operator()(const FilePath::Dns &dns) { parent->setDnsForm(dns); }
    void operator()(const FilePath::NvdimmNs &nvdimm_ns) { parent->setNvdimmNsForm(nvdimm_ns); }
    void operator()(const FilePath::RestService &rest_service) { parent->setRestServiceForm(rest_service); }
    void operator()(const FilePath::NvmeOfNs &nvme_of_ns) { parent->setNvmeOfNsForm(nvme_of_ns); }
    void operator()(const FilePath::Hd &hd) { parent->setHdForm(hd); }
    void operator()(const FilePath::CdRom &cd_rom) { parent->setCdRomForm(cd_rom); }
    void operator()(const FilePath::FilePath &file_path) { parent->setFilePathForm(file_path); }
    void operator()(const FilePath::Protocol &protocol) { parent->setProtocolForm(protocol); }
    void operator()(const FilePath::FirmwareFile &firmware_file) { parent->setFirmwareFileForm(firmware_file); }
    void operator()(const FilePath::FirmwareVolume &firmware_volume) { parent->setFirmwareVolumeForm(firmware_volume); }
    void operator()(const FilePath::RelativeOffsetRange &relative_offset_range) { parent->setRelativeOffsetRangeForm(relative_offset_range); }
    void operator()(const FilePath::RamDisk &ram_disk) { parent->setRamDiskForm(ram_disk); }
    void operator()(const FilePath::BootSpecification &boot_specification) { parent->setBootSpecificationForm(boot_specification); }
    void operator()(const FilePath::Vendor &vendor) { parent->setVendorForm(vendor); }
    void operator()(const FilePath::End &end) { parent->setEndForm(end._subtype); }
    void operator()(const FilePath::Unknown &unknown) { parent->setUnknownForm(unknown); }
};

void FilePathDialog::setFilePath(const FilePath::ANY *_file_path)
{
    resetForms();
    if(!_file_path)
    {
        update();
        return;
    }

    std::visit(FilePathVisitor(this), *_file_path);
    update();
}

// Hardware

void FilePathDialog::setPciForm(const FilePath::Pci &pci)
{
    ui->options->setCurrentIndex(FormIndex::PCI);
    ui->pci_function->setValue(static_cast<int>(pci.function));
    ui->pci_device->setValue(static_cast<int>(pci.device));
}

void FilePathDialog::setPccardForm(const FilePath::Pccard &pccard)
{
    ui->options->setCurrentIndex(FormIndex::PCCARD);
    ui->pccard_function_number->setValue(static_cast<int>(pccard.function_number));
}

void FilePathDialog::setMemoryMappedForm(const FilePath::MemoryMapped &memory_mapped)
{
    ui->options->setCurrentIndex(FormIndex::MEMORY_MAPPED);
    ui->memory_mapped_memory_type->setCurrentIndex(static_cast<int>(memory_mapped.memory_type));
    ui->memory_mapped_start_address->setText(toHex(memory_mapped.start_address));
    ui->memory_mapped_end_address->setText(toHex(memory_mapped.end_address));
}

void FilePathDialog::setControllerForm(const FilePath::Controller &controller)
{
    ui->options->setCurrentIndex(FormIndex::CONTROLLER);
    ui->controller_controller_number->setValue(static_cast<int>(controller.controller_number));
}

void FilePathDialog::setBmcForm(const FilePath::Bmc &bmc)
{
    ui->options->setCurrentIndex(FormIndex::BMC);
    ui->bmc_interface_type->setCurrentIndex(static_cast<int>(bmc.interface_type));
    ui->bmc_base_address->setText(toHex(bmc.base_address));
}

// ACPI

void FilePathDialog::setAcpiForm(const FilePath::Acpi &acpi)
{
    ui->options->setCurrentIndex(FormIndex::ACPI);
    ui->acpi_hid->setText(toHex(acpi.hid));
    ui->acpi_uid->setText(toHex(acpi.uid));
}

void FilePathDialog::setExpandedForm(const FilePath::Expanded &expanded)
{
    ui->options->setCurrentIndex(FormIndex::EXPANDED);
    ui->expanded_hid->setText(toHex(expanded.hid));
    ui->expanded_uid->setText(toHex(expanded.uid));
    ui->expanded_cid->setText(toHex(expanded.cid));
    ui->expanded_hidstr->setText(expanded.hidstr);
    ui->expanded_uidstr->setText(expanded.uidstr);
    ui->expanded_cidstr->setText(expanded.cidstr);
}

void FilePathDialog::setAdrForm(const FilePath::Adr &adr)
{
    ui->options->setCurrentIndex(FormIndex::ADR);
    ui->adr_adr->setText(toHex(adr.adr));
    ui->adr_additional_adr_format->setCurrentIndex(DataFormat::Base64);
    adr_additional_adr_format_index = 0;
    ui->adr_additional_adr->setPlainText(adr.additional_adr.toBase64());
}

void FilePathDialog::setNvdimmForm(const FilePath::Nvdimm &nvdimm)
{
    ui->options->setCurrentIndex(FormIndex::NVDIMM);
    ui->nvdimm_nfit_device_handle->setText(toHex(nvdimm.nfit_device_handle));
}

// Messaging

void FilePathDialog::setAtapiForm(const FilePath::Atapi &atapi)
{
    ui->options->setCurrentIndex(FormIndex::ATAPI);
    ui->atapi_primary->setChecked(atapi.primary);
    ui->atapi_slave->setChecked(atapi.slave);
    ui->atapi_lun->setValue(static_cast<int>(atapi.lun));
}

void FilePathDialog::setScsiForm(const FilePath::Scsi &scsi)
{
    ui->options->setCurrentIndex(FormIndex::SCSI);
    ui->scsi_pun->setValue(static_cast<int>(scsi.pun));
    ui->scsi_lun->setValue(static_cast<int>(scsi.lun));
}

void FilePathDialog::setFibreChannelForm(const FilePath::FibreChannel &fibre_channel)
{
    ui->options->setCurrentIndex(FormIndex::FIBRE_CHANNEL);
    ui->fibre_channel_reserved->setText(toHex(fibre_channel.reserved));
    ui->fibre_channel_world_wide_name->setText(toHex(fibre_channel.world_wide_name));
    ui->fibre_channel_lun->setText(toHex(fibre_channel.lun));
}

void FilePathDialog::setFirewireForm(const FilePath::Firewire &firewire)
{
    ui->options->setCurrentIndex(FormIndex::FIREWIRE);
    ui->firewire_reserved->setText(toHex(firewire.reserved));
    ui->firewire_guid->setText(toHex(firewire.guid));
}

void FilePathDialog::setUsbForm(const FilePath::Usb &usb)
{
    ui->options->setCurrentIndex(FormIndex::USB);
    ui->usb_parent_port_number->setValue(static_cast<int>(usb.parent_port_number));
    ui->usb_interface_number->setValue(static_cast<int>(usb.interface_number));
}

void FilePathDialog::setI2oForm(const FilePath::I2o &i2o)
{
    ui->options->setCurrentIndex(FormIndex::I2O);
    ui->i2o_tid->setValue(static_cast<int>(i2o.tid));
}

void FilePathDialog::setInfinibandForm(const FilePath::Infiniband &infiniband)
{
    ui->options->setCurrentIndex(FormIndex::INFINIBAND);
    ui->infiniband_resource_flags->setText(toHex(infiniband.resource_flags));
    ui->infiniband_port_gid->setText(infiniband.port_gid.toString());
    ui->infiniband_ioc_guid_service_id->setText(toHex(infiniband.ioc_guid_service_id));
    ui->infiniband_target_port_id->setText(toHex(infiniband.target_port_id));
    ui->infiniband_device_id->setText(toHex(infiniband.device_id));
}

void FilePathDialog::setMacAddressForm(const FilePath::MacAddress &mac_address)
{
    ui->options->setCurrentIndex(FormIndex::MAC_ADDRESS);
    ui->mac_address_address->setText(mac_address.address);
    ui->mac_address_if_type->setValue(static_cast<int>(mac_address.if_type));
}

void FilePathDialog::setIpv4Form(const FilePath::Ipv4 &ipv4)
{
    ui->options->setCurrentIndex(FormIndex::IPV4);
    ui->ipv4_local_ip_address->setText(ipv4.local_ip_address.toString());
    ui->ipv4_remote_ip_address->setText(ipv4.remote_ip_address.toString());
    ui->ipv4_local_port->setValue(static_cast<int>(ipv4.local_port));
    ui->ipv4_remote_port->setValue(static_cast<int>(ipv4.remote_port));
    ui->ipv4_protocol->setValue(static_cast<int>(ipv4.protocol));
    ui->ipv4_static_ip_address->setChecked(ipv4.static_ip_address);
    ui->ipv4_gateway_ip_address->setText(ipv4.gateway_ip_address.toString());
    ui->ipv4_subnet_mask->setText(ipv4.subnet_mask.toString());
}

void FilePathDialog::setIpv6Form(const FilePath::Ipv6 &ipv6)
{
    ui->options->setCurrentIndex(FormIndex::IPV6);
    ui->ipv6_local_ip_address->setText(ipv6.local_ip_address.toString());
    ui->ipv6_remote_ip_address->setText(ipv6.remote_ip_address.toString());
    ui->ipv6_local_port->setValue(static_cast<int>(ipv6.local_port));
    ui->ipv6_remote_port->setValue(static_cast<int>(ipv6.remote_port));
    ui->ipv6_protocol->setValue(static_cast<int>(ipv6.protocol));
    ui->ipv6_ip_address_origin->setCurrentIndex(static_cast<int>(ipv6.ip_address_origin));
    ui->ipv6_prefix_length->setValue(static_cast<int>(ipv6.prefix_length));
    ui->ipv6_gateway_ip_address->setText(ipv6.gateway_ip_address.toString());
}

void FilePathDialog::setUartForm(const FilePath::Uart &uart)
{
    ui->options->setCurrentIndex(FormIndex::UART);
    ui->uart_reserved->setText(toHex(uart.reserved));
    ui->uart_baud_rate->setText(toHex(uart.baud_rate));
    ui->uart_data_bits->setValue(static_cast<int>(uart.data_bits));
    ui->uart_parity->setCurrentIndex(static_cast<int>(uart.parity));
    ui->uart_stop_bits->setCurrentIndex(static_cast<int>(uart.stop_bits));
}

void FilePathDialog::setUsbClassForm(const FilePath::UsbClass &usb_class)
{
    ui->options->setCurrentIndex(FormIndex::USB_CLASS);
    ui->usb_class_vendor_id->setText(toHex(usb_class.vendor_id));
    ui->usb_class_product_id->setText(toHex(usb_class.product_id));
    ui->usb_class_device_class->setText(toHex(usb_class.device_class));
    ui->usb_class_device_subclass->setText(toHex(usb_class.device_subclass));
    ui->usb_class_device_protocol->setText(toHex(usb_class.device_protocol));
}

void FilePathDialog::setUsbWwidForm(const FilePath::UsbWwid &usb_wwid)
{
    ui->options->setCurrentIndex(FormIndex::USB_WWID);
    ui->usb_wwid_interface_number->setValue(static_cast<int>(usb_wwid.interface_number));
    ui->usb_wwid_device_vendor_id->setText(toHex(usb_wwid.device_vendor_id));
    ui->usb_wwid_device_product_id->setText(toHex(usb_wwid.device_product_id));
    ui->usb_wwid_serial_number->setText(usb_wwid.serial_number);
}

void FilePathDialog::setDeviceLogicalUnitForm(const FilePath::DeviceLogicalUnit &device_logical_unit)
{
    ui->options->setCurrentIndex(FormIndex::DEVICE_LOGICAL_UNIT);
    ui->device_logical_unit_lun->setValue(static_cast<int>(device_logical_unit.lun));
}

void FilePathDialog::setSataForm(const FilePath::Sata &sata)
{
    ui->options->setCurrentIndex(FormIndex::SATA);
    ui->sata_hba_port_number->setValue(static_cast<int>(sata.hba_port_number));
    ui->sata_port_multiplier_port_number->setValue(static_cast<int>(sata.port_multiplier_port_number));
    ui->sata_lun->setValue(static_cast<int>(sata.lun));
}

void FilePathDialog::setIscsiForm(const FilePath::Iscsi &iscsi)
{
    ui->options->setCurrentIndex(FormIndex::ISCSI);
    ui->iscsi_protocol->setValue(static_cast<int>(iscsi.protocol));
    ui->iscsi_options->setText(toHex(iscsi.options));
    ui->iscsi_lun->setText(toHex(iscsi.lun));
    ui->iscsi_target_portal_group->setValue(static_cast<int>(iscsi.target_portal_group));
    ui->iscsi_target_name->setText(iscsi.target_name);
}

void FilePathDialog::setVlanForm(const FilePath::Vlan &vlan)
{
    ui->options->setCurrentIndex(FormIndex::VLAN);
    ui->vlan_vlan_id->setValue(static_cast<int>(vlan.vlan_id));
}

void FilePathDialog::setFibreChannelExForm(const FilePath::FibreChannelEx &fibre_channel_ex)
{
    ui->options->setCurrentIndex(FormIndex::FIBRE_CHANNEL_EX);
    ui->fibre_channel_ex_reserved->setText(toHex(fibre_channel_ex.reserved));
    ui->fibre_channel_ex_world_wide_name->setText(toHex(fibre_channel_ex.world_wide_name));
    ui->fibre_channel_ex_lun->setText(toHex(fibre_channel_ex.lun));
}

void FilePathDialog::setSasExtendedMessagingForm(const FilePath::SasExtendedMessaging &sas_extended_messaging)
{
    ui->options->setCurrentIndex(FormIndex::SAS_EXTENDED_MESSAGING);
    ui->sas_extended_messaging_sas_address->setText(toHex(sas_extended_messaging.sas_address));
    ui->sas_extended_messaging_lun->setText(toHex(sas_extended_messaging.lun));
    ui->sas_extended_messaging_device_and_topology_info->setText(toHex(sas_extended_messaging.device_and_topology_info));
    ui->sas_extended_messaging_relative_target_port->setValue(static_cast<int>(sas_extended_messaging.relative_target_port));
}

void FilePathDialog::setNvmExpressNsForm(const FilePath::NvmExpressNs &nvm_express_ns)
{
    ui->options->setCurrentIndex(FormIndex::NVM_EXPRESS_NS);
    ui->nvm_express_ns_namespace_identifier->setText(toHex(nvm_express_ns.namespace_identifier));
    ui->nvm_express_ns_ieee_extended_unique_identifier->setText(toHex(nvm_express_ns.ieee_extended_unique_identifier));
}

void FilePathDialog::setUriForm(const FilePath::Uri &uri)
{
    ui->options->setCurrentIndex(FormIndex::URI);
    ui->uri_uri->setText(uri.uri.toDisplayString());
}

void FilePathDialog::setUfsForm(const FilePath::Ufs &ufs)
{
    ui->options->setCurrentIndex(FormIndex::UFS);
    ui->ufs_pun->setText(toHex(ufs.pun));
    ui->ufs_lun->setText(toHex(ufs.lun));
}

void FilePathDialog::setSdForm(const FilePath::Sd &sd)
{
    ui->options->setCurrentIndex(FormIndex::SD);
    ui->sd_slot_number->setValue(static_cast<int>(sd.slot_number));
}

void FilePathDialog::setBluetoothForm(const FilePath::Bluetooth &bluetooth)
{
    ui->options->setCurrentIndex(FormIndex::BLUETOOTH);
    ui->bluetooth_device_address->setText(bluetooth.device_address);
}

void FilePathDialog::setWiFiForm(const FilePath::WiFi &wi_fi)
{
    ui->options->setCurrentIndex(FormIndex::WI_FI);
    ui->wi_fi_ssid->setText(wi_fi.ssid);
}

void FilePathDialog::setEmmcForm(const FilePath::Emmc &emmc)
{
    ui->options->setCurrentIndex(FormIndex::EMMC);
    ui->emmc_slot_number->setValue(static_cast<int>(emmc.slot_number));
}

void FilePathDialog::setBluetoothleForm(const FilePath::Bluetoothle &bluetoothle)
{
    ui->options->setCurrentIndex(FormIndex::BLUETOOTHLE);
    ui->bluetoothle_device_address->setText(bluetoothle.device_address);
    ui->bluetoothle_address_type->setCurrentIndex(static_cast<int>(bluetoothle.address_type));
}

void FilePathDialog::setDnsForm(const FilePath::Dns &dns)
{
    ui->options->setCurrentIndex(FormIndex::DNS);
    ui->dns_ipv6->setChecked(dns.ipv6);
    ui->dns_data_format->setCurrentIndex(DataFormat::Base64);
    dns_data_format_index = 0;
    ui->dns_data->setPlainText(dns.data.toBase64());
}

void FilePathDialog::setNvdimmNsForm(const FilePath::NvdimmNs &nvdimm_ns)
{
    ui->options->setCurrentIndex(FormIndex::NVDIMM_NS);
    ui->nvdimm_ns_uuid->setText(nvdimm_ns.uuid.toString());
}

void FilePathDialog::setRestServiceForm(const FilePath::RestService &rest_service)
{
    ui->options->setCurrentIndex(FormIndex::REST_SERVICE);
    ui->rest_service_rest_service->setCurrentIndex(static_cast<int>(rest_service.rest_service));
    ui->rest_service_access_mode->setCurrentIndex(static_cast<int>(rest_service.access_mode));
    ui->rest_service_guid->setText(rest_service.guid.toString());
    ui->rest_service_data_format->setCurrentIndex(DataFormat::Base64);
    rest_service_data_format_index = 0;
    ui->rest_service_data->setPlainText(rest_service.data.toBase64());
}

void FilePathDialog::setNvmeOfNsForm(const FilePath::NvmeOfNs &nvme_of_ns)
{
    ui->options->setCurrentIndex(FormIndex::NVME_OF_NS);
    ui->nvme_of_ns_nidt->setValue(static_cast<int>(nvme_of_ns.nidt));
    ui->nvme_of_ns_nid->setText(nvme_of_ns.nid.toString());
    ui->nvme_of_ns_subsystem_nqn->setText(nvme_of_ns.subsystem_nqn);
}

// Media

void FilePathDialog::setHdForm(const FilePath::Hd &hd)
{
    ui->options->setCurrentIndex(FormIndex::HD);
    // Match existing drive
    for(int index = 0; index < ui->hd_disk->count() - 2; ++index)
    {
        const auto &drive_info = ui->hd_disk->itemData(index).value<DriveInfo>();
        if(static_cast<std::underlying_type_t<DriveInfo::SIGNATURE>>(drive_info.signature_type) == static_cast<std::underlying_type_t<EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE>>(hd.signature_type) && drive_info.partition == hd.partition_number)
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
                ui->hd_disk->setCurrentIndex(index);
                diskChoiceChanged(index);
                return;
            }
        }
    }

    // Custom
    ui->hd_disk->setCurrentIndex(ui->hd_disk->count() - 1);
    diskChoiceChanged(ui->hd_disk->currentIndex());

    ui->hd_partition_number->setValue(static_cast<int>(hd.partition_number));
    ui->hd_partition_start->setText(toHex(hd.partition_start));
    ui->hd_partition_size->setText(toHex(hd.partition_size));
    ui->hd_signature_type->setCurrentIndex(static_cast<int>(hd.signature_type));
    signatureTypeChoiceChanged(ui->hd_signature_type->currentIndex());
    if(hd.signature_type != EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE::NONE)
        ui->hd_partition_signature->setText(hd.partition_signature.toString());
}

void FilePathDialog::setCdRomForm(const FilePath::CdRom &cd_rom)
{
    ui->options->setCurrentIndex(FormIndex::CD_ROM);
    ui->cd_rom_boot_entry->setValue(static_cast<int>(cd_rom.boot_entry));
    ui->cd_rom_partition_start->setText(toHex(cd_rom.partition_start));
    ui->cd_rom_partition_size->setText(toHex(cd_rom.partition_size));
}

void FilePathDialog::setFilePathForm(const FilePath::FilePath &file_path)
{
    ui->options->setCurrentIndex(FormIndex::FILE_PATH);
    ui->file_path_path_name->setText(file_path.path_name);
}

void FilePathDialog::setProtocolForm(const FilePath::Protocol &protocol)
{
    ui->options->setCurrentIndex(FormIndex::PROTOCOL);
    ui->protocol_guid->setText(protocol.guid.toString());
}

void FilePathDialog::setFirmwareFileForm(const FilePath::FirmwareFile &firmware_file)
{
    ui->options->setCurrentIndex(FormIndex::FIRMWARE_FILE);
    ui->firmware_file_name->setText(firmware_file.name.toString());
}

void FilePathDialog::setFirmwareVolumeForm(const FilePath::FirmwareVolume &firmware_volume)
{
    ui->options->setCurrentIndex(FormIndex::FIRMWARE_VOLUME);
    ui->firmware_volume_name->setText(firmware_volume.name.toString());
}

void FilePathDialog::setRelativeOffsetRangeForm(const FilePath::RelativeOffsetRange &relative_offset_range)
{
    ui->options->setCurrentIndex(FormIndex::RELATIVE_OFFSET_RANGE);
    ui->relative_offset_range_reserved->setText(toHex(relative_offset_range.reserved));
    ui->relative_offset_range_starting_offset->setText(toHex(relative_offset_range.starting_offset));
    ui->relative_offset_range_ending_offset->setText(toHex(relative_offset_range.ending_offset));
}

void FilePathDialog::setRamDiskForm(const FilePath::RamDisk &ram_disk)
{
    ui->options->setCurrentIndex(FormIndex::RAM_DISK);
    ui->ram_disk_starting_address->setText(toHex(ram_disk.starting_address));
    ui->ram_disk_ending_address->setText(toHex(ram_disk.ending_address));
    ui->ram_disk_guid->setText(ram_disk.guid.toString());
    ui->ram_disk_disk_instance->setValue(static_cast<int>(ram_disk.disk_instance));
}

// BIOS

void FilePathDialog::setBootSpecificationForm(const FilePath::BootSpecification &boot_specification)
{
    ui->options->setCurrentIndex(FormIndex::BOOT_SPECIFICATION);
    ui->boot_specification_device_type->setText(toHex(boot_specification.device_type));
    ui->boot_specification_status_flag->setText(toHex(boot_specification.status_flag));
    ui->boot_specification_description->setText(boot_specification.description);
}

void FilePathDialog::setVendorForm(const FilePath::Vendor &vendor)
{
    ui->options->setCurrentIndex(FormIndex::VENDOR);
    VendorTypeIndex index{};
    switch(vendor._type)
    {
    case EFIBoot::File_path::HW::Vendor::TYPE:
        index = VendorTypeIndex::HW;
        break;

    case EFIBoot::File_path::MSG::Vendor::TYPE:
        index = VendorTypeIndex::MSG;
        break;

    case EFIBoot::File_path::MEDIA::Vendor::TYPE:
        index = VendorTypeIndex::MEDIA;
        break;

    default:
        index = VendorTypeIndex::HW;
        break;
    }

    ui->vendor_type->setCurrentIndex(static_cast<int>(index));
    ui->vendor_guid->setText(vendor.guid.toString(QUuid::WithoutBraces));
    ui->vendor_data_format->setCurrentIndex(DataFormat::Base64);
    vendor_data_format_index = 0;
    ui->vendor_data->setPlainText(vendor.data.toBase64());
}

void FilePathDialog::setEndForm(const uint8_t subtype)
{
    ui->options->setCurrentIndex(FormIndex::END);
    ui->end_subtype->setCurrentIndex(subtype == EFIBoot::File_path::END::Instance::SUBTYPE ? 0 : 1);
}

void FilePathDialog::setUnknownForm(const FilePath::Unknown &unknown)
{
    ui->options->setCurrentIndex(FormIndex::UNKNOWN);
    ui->unknown_type->setText(toHex(unknown._type));
    ui->unknown_subtype->setText(toHex(unknown._subtype));
    ui->unknown_data_format->setCurrentIndex(DataFormat::Base64);
    unknown_data_format_index = 0;
    ui->unknown_data->setPlainText(unknown.data.toBase64());
}

void FilePathDialog::resetForms()
{
    resetPciForm();
    resetPccardForm();
    resetMemoryMappedForm();
    resetControllerForm();
    resetBmcForm();
    resetAcpiForm();
    resetExpandedForm();
    resetAdrForm();
    resetNvdimmForm();
    resetAtapiForm();
    resetScsiForm();
    resetFibreChannelForm();
    resetFirewireForm();
    resetUsbForm();
    resetI2oForm();
    resetInfinibandForm();
    resetMacAddressForm();
    resetIpv4Form();
    resetIpv6Form();
    resetUartForm();
    resetUsbClassForm();
    resetUsbWwidForm();
    resetDeviceLogicalUnitForm();
    resetSataForm();
    resetIscsiForm();
    resetVlanForm();
    resetFibreChannelExForm();
    resetSasExtendedMessagingForm();
    resetNvmExpressNsForm();
    resetUriForm();
    resetUfsForm();
    resetSdForm();
    resetBluetoothForm();
    resetWiFiForm();
    resetEmmcForm();
    resetBluetoothleForm();
    resetDnsForm();
    resetNvdimmNsForm();
    resetRestServiceForm();
    resetNvmeOfNsForm();
    resetHdForm();
    resetCdRomForm();
    resetFilePathForm();
    resetProtocolForm();
    resetFirmwareFileForm();
    resetFirmwareVolumeForm();
    resetRelativeOffsetRangeForm();
    resetRamDiskForm();
    resetBootSpecificationForm();
    resetVendorForm();
    resetEndForm();
    resetUnknownForm();
}

// Hardware

void FilePathDialog::resetPciForm()
{
    ui->pci_function->clear();
    ui->pci_device->clear();
}

void FilePathDialog::resetPccardForm()
{
    ui->pccard_function_number->clear();
}

void FilePathDialog::resetMemoryMappedForm()
{
    ui->memory_mapped_memory_type->setCurrentIndex(0);
    ui->memory_mapped_start_address->clear();
    ui->memory_mapped_end_address->clear();
}

void FilePathDialog::resetControllerForm()
{
    ui->controller_controller_number->clear();
}

void FilePathDialog::resetBmcForm()
{
    ui->bmc_interface_type->setCurrentIndex(0);
    ui->bmc_base_address->clear();
}

// ACPI

void FilePathDialog::resetAcpiForm()
{
    ui->acpi_hid->clear();
    ui->acpi_uid->clear();
}

void FilePathDialog::resetExpandedForm()
{
    ui->expanded_hid->clear();
    ui->expanded_uid->clear();
    ui->expanded_cid->clear();
    ui->expanded_hidstr->clear();
    ui->expanded_uidstr->clear();
    ui->expanded_cidstr->clear();
}

void FilePathDialog::resetAdrForm()
{
    ui->adr_adr->clear();
    ui->adr_additional_adr_format->setCurrentIndex(DataFormat::Base64);
    adr_additional_adr_format_index = 0;
    ui->adr_additional_adr->clear();
}

void FilePathDialog::resetNvdimmForm()
{
    ui->nvdimm_nfit_device_handle->clear();
}

// Messaging

void FilePathDialog::resetAtapiForm()
{
    ui->atapi_primary->setChecked(false);
    ui->atapi_slave->setChecked(false);
    ui->atapi_lun->clear();
}

void FilePathDialog::resetScsiForm()
{
    ui->scsi_pun->clear();
    ui->scsi_lun->clear();
}

void FilePathDialog::resetFibreChannelForm()
{
    ui->fibre_channel_reserved->clear();
    ui->fibre_channel_world_wide_name->clear();
    ui->fibre_channel_lun->clear();
}

void FilePathDialog::resetFirewireForm()
{
    ui->firewire_reserved->clear();
    ui->firewire_guid->clear();
}

void FilePathDialog::resetUsbForm()
{
    ui->usb_parent_port_number->clear();
    ui->usb_interface_number->clear();
}

void FilePathDialog::resetI2oForm()
{
    ui->i2o_tid->clear();
}

void FilePathDialog::resetInfinibandForm()
{
    ui->infiniband_resource_flags->clear();
    ui->infiniband_port_gid->clear();
    ui->infiniband_ioc_guid_service_id->clear();
    ui->infiniband_target_port_id->clear();
    ui->infiniband_device_id->clear();
}

void FilePathDialog::resetMacAddressForm()
{
    ui->mac_address_address->clear();
    ui->mac_address_if_type->clear();
}

void FilePathDialog::resetIpv4Form()
{
    ui->ipv4_local_ip_address->clear();
    ui->ipv4_remote_ip_address->clear();
    ui->ipv4_local_port->clear();
    ui->ipv4_remote_port->clear();
    ui->ipv4_protocol->clear();
    ui->ipv4_static_ip_address->setChecked(false);
    ui->ipv4_gateway_ip_address->clear();
    ui->ipv4_subnet_mask->clear();
}

void FilePathDialog::resetIpv6Form()
{
    ui->ipv6_local_ip_address->clear();
    ui->ipv6_remote_ip_address->clear();
    ui->ipv6_local_port->clear();
    ui->ipv6_remote_port->clear();
    ui->ipv6_protocol->clear();
    ui->ipv6_ip_address_origin->setCurrentIndex(0);
    ui->ipv6_prefix_length->clear();
    ui->ipv6_gateway_ip_address->clear();
}

void FilePathDialog::resetUartForm()
{
    ui->uart_reserved->clear();
    ui->uart_baud_rate->clear();
    ui->uart_data_bits->clear();
    ui->uart_parity->setCurrentIndex(0);
    ui->uart_stop_bits->setCurrentIndex(0);
}

void FilePathDialog::resetUsbClassForm()
{
    ui->usb_class_vendor_id->clear();
    ui->usb_class_product_id->clear();
    ui->usb_class_device_class->clear();
    ui->usb_class_device_subclass->clear();
    ui->usb_class_device_protocol->clear();
}

void FilePathDialog::resetUsbWwidForm()
{
    ui->usb_wwid_interface_number->clear();
    ui->usb_wwid_device_vendor_id->clear();
    ui->usb_wwid_device_product_id->clear();
    ui->usb_wwid_serial_number->clear();
}

void FilePathDialog::resetDeviceLogicalUnitForm()
{
    ui->device_logical_unit_lun->clear();
}

void FilePathDialog::resetSataForm()
{
    ui->sata_hba_port_number->clear();
    ui->sata_port_multiplier_port_number->clear();
    ui->sata_lun->clear();
}

void FilePathDialog::resetIscsiForm()
{
    ui->iscsi_protocol->clear();
    ui->iscsi_options->clear();
    ui->iscsi_lun->clear();
    ui->iscsi_target_portal_group->clear();
    ui->iscsi_target_name->clear();
}

void FilePathDialog::resetVlanForm()
{
    ui->vlan_vlan_id->clear();
}

void FilePathDialog::resetFibreChannelExForm()
{
    ui->fibre_channel_ex_reserved->clear();
    ui->fibre_channel_ex_world_wide_name->clear();
    ui->fibre_channel_ex_lun->clear();
}

void FilePathDialog::resetSasExtendedMessagingForm()
{
    ui->sas_extended_messaging_sas_address->clear();
    ui->sas_extended_messaging_lun->clear();
    ui->sas_extended_messaging_device_and_topology_info->clear();
    ui->sas_extended_messaging_relative_target_port->clear();
}

void FilePathDialog::resetNvmExpressNsForm()
{
    ui->nvm_express_ns_namespace_identifier->clear();
    ui->nvm_express_ns_ieee_extended_unique_identifier->clear();
}

void FilePathDialog::resetUriForm()
{
    ui->uri_uri->clear();
}

void FilePathDialog::resetUfsForm()
{
    ui->ufs_pun->clear();
    ui->ufs_lun->clear();
}

void FilePathDialog::resetSdForm()
{
    ui->sd_slot_number->clear();
}

void FilePathDialog::resetBluetoothForm()
{
    ui->bluetooth_device_address->clear();
}

void FilePathDialog::resetWiFiForm()
{
    ui->wi_fi_ssid->clear();
}

void FilePathDialog::resetEmmcForm()
{
    ui->emmc_slot_number->clear();
}

void FilePathDialog::resetBluetoothleForm()
{
    ui->bluetoothle_device_address->clear();
    ui->bluetoothle_address_type->setCurrentIndex(0);
}

void FilePathDialog::resetDnsForm()
{
    ui->dns_ipv6->setChecked(false);
    ui->dns_data_format->setCurrentIndex(DataFormat::Base64);
    dns_data_format_index = 0;
    ui->dns_data->clear();
}

void FilePathDialog::resetNvdimmNsForm()
{
    ui->nvdimm_ns_uuid->clear();
}

void FilePathDialog::resetRestServiceForm()
{
    ui->rest_service_rest_service->setCurrentIndex(0);
    ui->rest_service_access_mode->setCurrentIndex(0);
    ui->rest_service_guid->clear();
    ui->rest_service_data_format->setCurrentIndex(DataFormat::Base64);
    rest_service_data_format_index = 0;
    ui->rest_service_data->clear();
}

void FilePathDialog::resetNvmeOfNsForm()
{
    ui->nvme_of_ns_nidt->clear();
    ui->nvme_of_ns_nid->clear();
    ui->nvme_of_ns_subsystem_nqn->clear();
}

// Media

void FilePathDialog::resetHdForm()
{
    refreshDiskCombo(false);
    signatureTypeChoiceChanged(ui->hd_signature_type->currentIndex());
    ui->hd_partition_number->clear();
    ui->hd_partition_start->clear();
    ui->hd_partition_size->clear();
    ui->hd_signature_type->setCurrentIndex(0);
}

void FilePathDialog::resetCdRomForm()
{
    ui->cd_rom_boot_entry->clear();
    ui->cd_rom_partition_start->clear();
    ui->cd_rom_partition_size->clear();
}

void FilePathDialog::resetFilePathForm()
{
    ui->file_path_path_name->clear();
}

void FilePathDialog::resetProtocolForm()
{
    ui->protocol_guid->clear();
}

void FilePathDialog::resetFirmwareFileForm()
{
    ui->firmware_file_name->clear();
}

void FilePathDialog::resetFirmwareVolumeForm()
{
    ui->firmware_volume_name->clear();
}

void FilePathDialog::resetRelativeOffsetRangeForm()
{
    ui->relative_offset_range_reserved->clear();
    ui->relative_offset_range_starting_offset->clear();
    ui->relative_offset_range_ending_offset->clear();
}

void FilePathDialog::resetRamDiskForm()
{
    ui->ram_disk_starting_address->clear();
    ui->ram_disk_ending_address->clear();
    ui->ram_disk_guid->clear();
    ui->ram_disk_disk_instance->clear();
}

// BIOS

void FilePathDialog::resetBootSpecificationForm()
{
    ui->boot_specification_device_type->clear();
    ui->boot_specification_status_flag->clear();
    ui->boot_specification_description->clear();
}

void FilePathDialog::resetVendorForm()
{
    ui->vendor_guid->clear();
    ui->vendor_type->setCurrentIndex(0);
    ui->vendor_data_format->setCurrentIndex(DataFormat::Base64);
    vendor_data_format_index = 0;
    ui->vendor_data->clear();
}

void FilePathDialog::resetEndForm()
{
    ui->end_subtype->setCurrentIndex(0);
}

void FilePathDialog::resetUnknownForm()
{
    ui->unknown_type->clear();
    ui->unknown_subtype->clear();
    ui->unknown_data_format->setCurrentIndex(DataFormat::Base64);
    unknown_data_format_index = 0;
    ui->unknown_data->clear();
}

void FilePathDialog::refreshDiskCombo(bool force)
{
    ui->hd_disk->clear();
    const auto drives = DriveInfo::getAll(force);
    for(const auto &drive: drives)
    {
        QVariant item;
        item.setValue(drive);
        ui->hd_disk->addItem(drive.name, item);
    }

    ui->hd_disk->insertSeparator(ui->hd_disk->count());
    ui->hd_disk->addItem("Custom");

    int index = ui->hd_disk->count() - 1;
    ui->hd_disk->setCurrentIndex(index);
    diskChoiceChanged(index);
}

QByteArray FilePathDialog::getData(const QPlainTextEdit &data, int index) const
{
    switch(static_cast<DataFormat>(index))
    {
    case DataFormat::Base64:
        return QByteArray::fromBase64(data.toPlainText().toUtf8());

    case DataFormat::Utf16:
        return fromUnicode(data.toPlainText(), "UTF-16");

    case DataFormat::Utf8:
        return fromUnicode(data.toPlainText(), "UTF-8");

    case DataFormat::Hex:
        return QByteArray::fromHex(data.toPlainText().toUtf8());
    }

    return {};
}

void FilePathDialog::dataFormatChanged(int &index, int new_index, QPlainTextEdit &data, QComboBox &format)
{
    bool success = false;
    QByteArray input = getData(data, index);
    QString output;
    switch(static_cast<DataFormat>(new_index))
    {
    case DataFormat::Base64:
        output = input.toBase64();
        success = true;
        break;

    case DataFormat::Utf16:
        if(static_cast<uint>(input.size()) % sizeof(char16_t) == 0)
            success = toUnicode(output, input, "UTF-16");

        break;

    case DataFormat::Utf8:
        success = toUnicode(output, input, "UTF-8");
        break;

    case DataFormat::Hex:
        output = input.toHex();
        success = true;
        break;
    }

    if(output.contains(QChar(0)))
        success = false;

    if(!success)
    {
        QMessageBox::critical(this, qApp->applicationName(), tr("Couldn't change data format!"));
        format.setCurrentIndex(index);
        return;
    }

    data.setPlainText(output);
    index = new_index;
}

void FilePathDialog::resetDiskCombo()
{
    refreshDiskCombo(true);
}

void FilePathDialog::diskChoiceChanged(int index)
{
    bool disabled = index + 1 != ui->hd_disk->count();
    ui->hd_signature_type->setDisabled(disabled);
    ui->hd_partition_signature->setDisabled(disabled);
    ui->hd_partition_number->setDisabled(disabled);
    ui->hd_partition_start->setDisabled(disabled);
    ui->hd_partition_size->setDisabled(disabled);

    if(index + 1 == ui->hd_disk->count())
        return;

    ui->hd_signature_type->setCurrentIndex(0);
    signatureTypeChoiceChanged(ui->hd_signature_type->currentIndex());
    ui->hd_partition_number->clear();
    ui->hd_partition_start->clear();
    ui->hd_partition_size->clear();

    const auto &driveinfo = ui->hd_disk->itemData(index).value<DriveInfo>();
    switch(static_cast<DriveInfo::SIGNATURE>(driveinfo.signature_type))
    {
    case DriveInfo::SIGNATURE::NONE:
        ui->hd_signature_type->setCurrentIndex(0);
        break;

    case DriveInfo::SIGNATURE::MBR:
        ui->hd_signature_type->setCurrentIndex(1);
        break;

    case DriveInfo::SIGNATURE::GUID:
        ui->hd_signature_type->setCurrentIndex(2);
        break;
    }

    signatureTypeChoiceChanged(ui->hd_signature_type->currentIndex());
    if(driveinfo.signature_type != DriveInfo::SIGNATURE::NONE)
        ui->hd_partition_signature->setText(driveinfo.signature.toString());

    ui->hd_partition_number->setValue(static_cast<int>(driveinfo.partition));
    ui->hd_partition_start->setText(toHex(driveinfo.start));
    ui->hd_partition_size->setText(toHex(driveinfo.size));
}

void FilePathDialog::signatureTypeChoiceChanged(int index)
{
    switch(static_cast<DriveInfo::SIGNATURE>(index))
    {
    case DriveInfo::SIGNATURE::NONE:
        ui->hd_partition_signature->setDisabled(true);
        ui->hd_partition_signature->setInputMask("");
        ui->hd_partition_signature->clear();
        ui->hd_partition_number->setMaximum(INT_MAX);
        break;

    case DriveInfo::SIGNATURE::MBR:
        ui->hd_partition_signature->setDisabled(ui->hd_disk->currentIndex() + 1 != ui->hd_disk->count());
        ui->hd_partition_signature->setInputMask("<\\0\\xHHHHHHHH;_");
        ui->hd_partition_signature->clear();
        ui->hd_partition_number->setMaximum(4);
        break;

    case DriveInfo::SIGNATURE::GUID:
        ui->hd_partition_signature->setDisabled(ui->hd_disk->currentIndex() + 1 != ui->hd_disk->count());
        ui->hd_partition_signature->setInputMask("<HHHHHHHH-HHHH-HHHH-HHHH-HHHHHHHHHHHH;_");
        ui->hd_partition_signature->clear();
        ui->hd_partition_number->setMaximum(128);
        break;
    }
}

void FilePathDialog::AdrAdditionalAdrChanged(int index)
{
    return dataFormatChanged(adr_additional_adr_format_index, index, *ui->adr_additional_adr, *ui->adr_additional_adr_format);
}

void FilePathDialog::DnsDataChanged(int index)
{
    return dataFormatChanged(dns_data_format_index, index, *ui->dns_data, *ui->dns_data_format);
}

void FilePathDialog::RestServiceDataChanged(int index)
{
    return dataFormatChanged(rest_service_data_format_index, index, *ui->rest_service_data, *ui->rest_service_data_format);
}

void FilePathDialog::VendorDataFormatChanged(int index)
{
    dataFormatChanged(vendor_data_format_index, index, *ui->vendor_data, *ui->vendor_data_format);
}

void FilePathDialog::UnknownDataFormatChanged(int index)
{
    dataFormatChanged(unknown_data_format_index, index, *ui->unknown_data, *ui->unknown_data_format);
}
