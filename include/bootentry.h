// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QHostAddress>
#include <QMetaType>
#include <QString>
#include <QUrl>
#include <QUuid>
#include <QVariant>
#include <QVector>

#include "efiboot.h"

namespace FilePath
{

/*
   Hardware
   This Device Path defines how a device is attached to the resource domain of a system, where resource domain is simply the shared memory, memory mapped I/O, and I/O space of the system.
*/

/*
   PCI
   The Device Path for PCI defines the path to the PCI configuration space address for a PCI device.
*/
class Pci
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "PCI";

private:
    mutable QString _string = {};

public:
    uint8_t function = {};
    uint8_t device = {};

public:
    Pci() = default;
    Pci(const EFIBoot::File_path::HW::Pci &pci);
    EFIBoot::File_path::HW::Pci toEFIBootFilePath() const;

    static std::optional<Pci> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   PCCARD
   PCCARD Settings.
*/
class Pccard
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "PCCARD";

private:
    mutable QString _string = {};

public:
    uint8_t function_number = {};

public:
    Pccard() = default;
    Pccard(const EFIBoot::File_path::HW::Pccard &pccard);
    EFIBoot::File_path::HW::Pccard toEFIBootFilePath() const;

    static std::optional<Pccard> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Memory Mapped
   Memory Mapped Settings.
*/
class MemoryMapped
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "MEMORY_MAPPED";

private:
    mutable QString _string = {};

public:
    EFIBoot::File_path::HW::Memory_mapped::MEMORY_TYPE memory_type = {};
    uint64_t start_address = {};
    uint64_t end_address = {};

public:
    MemoryMapped() = default;
    MemoryMapped(const EFIBoot::File_path::HW::Memory_mapped &memory_mapped);
    EFIBoot::File_path::HW::Memory_mapped toEFIBootFilePath() const;

    static std::optional<MemoryMapped> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Controller
   Controller settings.
*/
class Controller
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "CONTROLLER";

private:
    mutable QString _string = {};

public:
    uint32_t controller_number = {};

public:
    Controller() = default;
    Controller(const EFIBoot::File_path::HW::Controller &controller);
    EFIBoot::File_path::HW::Controller toEFIBootFilePath() const;

    static std::optional<Controller> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   BMC
   The Device Path for a Baseboard Management Controller (BMC) host interface.
*/
class Bmc
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "BMC";

private:
    mutable QString _string = {};

public:
    EFIBoot::File_path::HW::Bmc::INTERFACE_TYPE interface_type = {};
    uint64_t base_address = {};

public:
    Bmc() = default;
    Bmc(const EFIBoot::File_path::HW::Bmc &bmc);
    EFIBoot::File_path::HW::Bmc toEFIBootFilePath() const;

    static std::optional<Bmc> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   ACPI
   This Device Path is used to describe devices whose enumeration is not described in an industry-standard fashion. These devices must be described using ACPI AML in the ACPI name space; this Device Path is a linkage to the ACPI name space.
*/

/*
   ACPI
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
class Acpi
{
public:
    static constexpr auto TYPE = "ACPI";
    static constexpr auto SUBTYPE = "ACPI";

private:
    mutable QString _string = {};

public:
    uint32_t hid = {};
    uint32_t uid = {};

public:
    Acpi() = default;
    Acpi(const EFIBoot::File_path::ACPI::Acpi &acpi);
    EFIBoot::File_path::ACPI::Acpi toEFIBootFilePath() const;

    static std::optional<Acpi> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Expanded
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
class Expanded
{
public:
    static constexpr auto TYPE = "ACPI";
    static constexpr auto SUBTYPE = "EXPANDED";

private:
    mutable QString _string = {};

public:
    uint32_t hid = {};
    uint32_t uid = {};
    uint32_t cid = {};
    QString hidstr = {};
    QString uidstr = {};
    QString cidstr = {};

public:
    Expanded() = default;
    Expanded(const EFIBoot::File_path::ACPI::Expanded &expanded);
    EFIBoot::File_path::ACPI::Expanded toEFIBootFilePath() const;

    static std::optional<Expanded> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   ADR
   The ADR device path is used to contain video output device attributes to support the Graphics Output Protocol.
*/
class Adr
{
public:
    static constexpr auto TYPE = "ACPI";
    static constexpr auto SUBTYPE = "ADR";

private:
    mutable QString _string = {};

public:
    uint32_t adr = {};
    QByteArray additional_adr = {};

public:
    Adr() = default;
    Adr(const EFIBoot::File_path::ACPI::Adr &adr);
    EFIBoot::File_path::ACPI::Adr toEFIBootFilePath() const;

    static std::optional<Adr> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   NVDIMM
   This device path describes an NVDIMM device using the ACPI 6.0 specification defined NFIT Device Handle as the identifier.
*/
class Nvdimm
{
public:
    static constexpr auto TYPE = "ACPI";
    static constexpr auto SUBTYPE = "NVDIMM";

private:
    mutable QString _string = {};

public:
    uint32_t nfit_device_handle = {};

public:
    Nvdimm() = default;
    Nvdimm(const EFIBoot::File_path::ACPI::Nvdimm &nvdimm);
    EFIBoot::File_path::ACPI::Nvdimm toEFIBootFilePath() const;

    static std::optional<Nvdimm> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Messaging
   This Device Path is used to describe the connection of devices outside the resource domain of the system. This Device Path can describe physical messaging information such as a SCSI ID, or abstract information such as networking protocol IP addresses.
*/

/*
   ATAPI
   ATAPI Settings.
*/
class Atapi
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "ATAPI";

private:
    mutable QString _string = {};

public:
    bool primary = {};
    bool slave = {};
    uint16_t lun = {};

public:
    Atapi() = default;
    Atapi(const EFIBoot::File_path::MSG::Atapi &atapi);
    EFIBoot::File_path::MSG::Atapi toEFIBootFilePath() const;

    static std::optional<Atapi> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   SCSI
   SCSI Settings.
*/
class Scsi
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "SCSI";

private:
    mutable QString _string = {};

public:
    uint16_t pun = {};
    uint16_t lun = {};

public:
    Scsi() = default;
    Scsi(const EFIBoot::File_path::MSG::Scsi &scsi);
    EFIBoot::File_path::MSG::Scsi toEFIBootFilePath() const;

    static std::optional<Scsi> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Fibre Channel
   Fibre Channel Settings
*/
class FibreChannel
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "FIBRE_CHANNEL";

private:
    mutable QString _string = {};

public:
    uint32_t reserved = {};
    uint64_t world_wide_name = {};
    uint64_t lun = {};

public:
    FibreChannel() = default;
    FibreChannel(const EFIBoot::File_path::MSG::Fibre_channel &fibre_channel);
    EFIBoot::File_path::MSG::Fibre_channel toEFIBootFilePath() const;

    static std::optional<FibreChannel> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Firewire
   Firewire Settings.
*/
class Firewire
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "FIREWIRE";

private:
    mutable QString _string = {};

public:
    uint32_t reserved = {};
    uint64_t guid = {};

public:
    Firewire() = default;
    Firewire(const EFIBoot::File_path::MSG::Firewire &firewire);
    EFIBoot::File_path::MSG::Firewire toEFIBootFilePath() const;

    static std::optional<Firewire> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   USB
   USB settings.
*/
class Usb
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "USB";

private:
    mutable QString _string = {};

public:
    uint8_t parent_port_number = {};
    uint8_t interface_number = {};

public:
    Usb() = default;
    Usb(const EFIBoot::File_path::MSG::Usb &usb);
    EFIBoot::File_path::MSG::Usb toEFIBootFilePath() const;

    static std::optional<Usb> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   I2O
   I2O Settings
*/
class I2o
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "I2O";

private:
    mutable QString _string = {};

public:
    uint32_t tid = {};

public:
    I2o() = default;
    I2o(const EFIBoot::File_path::MSG::I2o &i2o);
    EFIBoot::File_path::MSG::I2o toEFIBootFilePath() const;

    static std::optional<I2o> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   InfiniBand
   InfiniBand Settings.
*/
class Infiniband
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "INFINIBAND";

private:
    mutable QString _string = {};

public:
    uint32_t resource_flags = {};
    QUuid port_gid = {};
    uint64_t ioc_guid_service_id = {};
    uint64_t target_port_id = {};
    uint64_t device_id = {};

public:
    Infiniband() = default;
    Infiniband(const EFIBoot::File_path::MSG::Infiniband &infiniband);
    EFIBoot::File_path::MSG::Infiniband toEFIBootFilePath() const;

    static std::optional<Infiniband> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   MAC Address
   MAC settings.
*/
class MacAddress
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "MAC_ADDRESS";

private:
    mutable QString _string = {};

public:
    QString address = {};
    uint8_t if_type = {};

public:
    MacAddress() = default;
    MacAddress(const EFIBoot::File_path::MSG::Mac_address &mac_address);
    EFIBoot::File_path::MSG::Mac_address toEFIBootFilePath() const;

    static std::optional<MacAddress> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   IPv4
   IPv4 settings.
*/
class Ipv4
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "IPV4";

private:
    mutable QString _string = {};

public:
    QHostAddress local_ip_address = {};
    QHostAddress remote_ip_address = {};
    uint16_t local_port = {};
    uint16_t remote_port = {};
    uint16_t protocol = {};
    bool static_ip_address = {};
    QHostAddress gateway_ip_address = {};
    QHostAddress subnet_mask = {};

public:
    Ipv4() = default;
    Ipv4(const EFIBoot::File_path::MSG::Ipv4 &ipv4);
    EFIBoot::File_path::MSG::Ipv4 toEFIBootFilePath() const;

    static std::optional<Ipv4> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   IPv6
   IPv6 settings.
*/
class Ipv6
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "IPV6";

private:
    mutable QString _string = {};

public:
    QHostAddress local_ip_address = {};
    QHostAddress remote_ip_address = {};
    uint16_t local_port = {};
    uint16_t remote_port = {};
    uint16_t protocol = {};
    EFIBoot::File_path::MSG::Ipv6::IP_ADDRESS_ORIGIN ip_address_origin = {};
    uint8_t prefix_length = {};
    QHostAddress gateway_ip_address = {};

public:
    Ipv6() = default;
    Ipv6(const EFIBoot::File_path::MSG::Ipv6 &ipv6);
    EFIBoot::File_path::MSG::Ipv6 toEFIBootFilePath() const;

    static std::optional<Ipv6> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   UART
   UART Settings.
*/
class Uart
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "UART";

private:
    mutable QString _string = {};

public:
    uint32_t reserved = {};
    uint64_t baud_rate = {};
    uint8_t data_bits = {};
    EFIBoot::File_path::MSG::Uart::PARITY parity = {};
    EFIBoot::File_path::MSG::Uart::STOP_BITS stop_bits = {};

public:
    Uart() = default;
    Uart(const EFIBoot::File_path::MSG::Uart &uart);
    EFIBoot::File_path::MSG::Uart toEFIBootFilePath() const;

    static std::optional<Uart> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   USB Class
   USB Class Settings.
*/
class UsbClass
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "USB_CLASS";

private:
    mutable QString _string = {};

public:
    uint16_t vendor_id = {};
    uint16_t product_id = {};
    uint8_t device_class = {};
    uint8_t device_subclass = {};
    uint8_t device_protocol = {};

public:
    UsbClass() = default;
    UsbClass(const EFIBoot::File_path::MSG::Usb_class &usb_class);
    EFIBoot::File_path::MSG::Usb_class toEFIBootFilePath() const;

    static std::optional<UsbClass> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   USB WWID
   This device path describes a USB device using its serial number.
*/
class UsbWwid
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "USB_WWID";

private:
    mutable QString _string = {};

public:
    uint16_t interface_number = {};
    uint16_t device_vendor_id = {};
    uint16_t device_product_id = {};
    QString serial_number = {};

public:
    UsbWwid() = default;
    UsbWwid(const EFIBoot::File_path::MSG::Usb_wwid &usb_wwid);
    EFIBoot::File_path::MSG::Usb_wwid toEFIBootFilePath() const;

    static std::optional<UsbWwid> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Device Logical Unit
   Device Logical Unit Settings.
*/
class DeviceLogicalUnit
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "DEVICE_LOGICAL_UNIT";

private:
    mutable QString _string = {};

public:
    uint8_t lun = {};

public:
    DeviceLogicalUnit() = default;
    DeviceLogicalUnit(const EFIBoot::File_path::MSG::Device_logical_unit &device_logical_unit);
    EFIBoot::File_path::MSG::Device_logical_unit toEFIBootFilePath() const;

    static std::optional<DeviceLogicalUnit> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   SATA
   SATA settings.
*/
class Sata
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "SATA";

private:
    mutable QString _string = {};

public:
    uint16_t hba_port_number = {};
    uint16_t port_multiplier_port_number = {};
    uint16_t lun = {};

public:
    Sata() = default;
    Sata(const EFIBoot::File_path::MSG::Sata &sata);
    EFIBoot::File_path::MSG::Sata toEFIBootFilePath() const;

    static std::optional<Sata> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   iSCSI
   iSCSI Settings.
*/
class Iscsi
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "ISCSI";

private:
    mutable QString _string = {};

public:
    uint16_t protocol = {};
    uint16_t options = {};
    uint64_t lun = {};
    uint16_t target_portal_group = {};
    QString target_name = {};

public:
    Iscsi() = default;
    Iscsi(const EFIBoot::File_path::MSG::Iscsi &iscsi);
    EFIBoot::File_path::MSG::Iscsi toEFIBootFilePath() const;

    static std::optional<Iscsi> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   VLAN
   VLAN Settings.
*/
class Vlan
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "VLAN";

private:
    mutable QString _string = {};

public:
    uint16_t vlan_id = {};

public:
    Vlan() = default;
    Vlan(const EFIBoot::File_path::MSG::Vlan &vlan);
    EFIBoot::File_path::MSG::Vlan toEFIBootFilePath() const;

    static std::optional<Vlan> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Fibre Channel Ex
   The Fibre Channel Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
class FibreChannelEx
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "FIBRE_CHANNEL_EX";

private:
    mutable QString _string = {};

public:
    uint32_t reserved = {};
    uint64_t world_wide_name = {};
    uint64_t lun = {};

public:
    FibreChannelEx() = default;
    FibreChannelEx(const EFIBoot::File_path::MSG::Fibre_channel_ex &fibre_channel_ex);
    EFIBoot::File_path::MSG::Fibre_channel_ex toEFIBootFilePath() const;

    static std::optional<FibreChannelEx> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   SAS Extended Messaging
   The SAS Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
class SasExtendedMessaging
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "SAS_EXTENDED_MESSAGING";

private:
    mutable QString _string = {};

public:
    uint64_t sas_address = {};
    uint64_t lun = {};
    uint16_t device_and_topology_info = {};
    uint16_t relative_target_port = {};

public:
    SasExtendedMessaging() = default;
    SasExtendedMessaging(const EFIBoot::File_path::MSG::Sas_extended_messaging &sas_extended_messaging);
    EFIBoot::File_path::MSG::Sas_extended_messaging toEFIBootFilePath() const;

    static std::optional<SasExtendedMessaging> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   NVM Express NS
   NVM Express Namespace Settings.
*/
class NvmExpressNs
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "NVM_EXPRESS_NS";

private:
    mutable QString _string = {};

public:
    uint32_t namespace_identifier = {};
    uint64_t ieee_extended_unique_identifier = {};

public:
    NvmExpressNs() = default;
    NvmExpressNs(const EFIBoot::File_path::MSG::Nvm_express_ns &nvm_express_ns);
    EFIBoot::File_path::MSG::Nvm_express_ns toEFIBootFilePath() const;

    static std::optional<NvmExpressNs> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   URI
   Refer to RFC 3986 for details on the URI contents.
*/
class Uri
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "URI";

private:
    mutable QString _string = {};

public:
    QUrl uri = {};

public:
    Uri() = default;
    Uri(const EFIBoot::File_path::MSG::Uri &uri);
    EFIBoot::File_path::MSG::Uri toEFIBootFilePath() const;

    static std::optional<Uri> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   UFS
   UFS Settings.
*/
class Ufs
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "UFS";

private:
    mutable QString _string = {};

public:
    uint8_t pun = {};
    uint8_t lun = {};

public:
    Ufs() = default;
    Ufs(const EFIBoot::File_path::MSG::Ufs &ufs);
    EFIBoot::File_path::MSG::Ufs toEFIBootFilePath() const;

    static std::optional<Ufs> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   SD
   SD Settings.
*/
class Sd
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "SD";

private:
    mutable QString _string = {};

public:
    uint8_t slot_number = {};

public:
    Sd() = default;
    Sd(const EFIBoot::File_path::MSG::Sd &sd);
    EFIBoot::File_path::MSG::Sd toEFIBootFilePath() const;

    static std::optional<Sd> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Bluetooth
   EFI Bluetooth Settings.
*/
class Bluetooth
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "BLUETOOTH";

private:
    mutable QString _string = {};

public:
    QString device_address = {};

public:
    Bluetooth() = default;
    Bluetooth(const EFIBoot::File_path::MSG::Bluetooth &bluetooth);
    EFIBoot::File_path::MSG::Bluetooth toEFIBootFilePath() const;

    static std::optional<Bluetooth> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Wi-Fi
   Wi-Fi Settings.
*/
class WiFi
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "WI_FI";

private:
    mutable QString _string = {};

public:
    QString ssid = {};

public:
    WiFi() = default;
    WiFi(const EFIBoot::File_path::MSG::Wi_fi &wi_fi);
    EFIBoot::File_path::MSG::Wi_fi toEFIBootFilePath() const;

    static std::optional<WiFi> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   eMMC
   Embedded Multi-Media Card Settings.
*/
class Emmc
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "EMMC";

private:
    mutable QString _string = {};

public:
    uint8_t slot_number = {};

public:
    Emmc() = default;
    Emmc(const EFIBoot::File_path::MSG::Emmc &emmc);
    EFIBoot::File_path::MSG::Emmc toEFIBootFilePath() const;

    static std::optional<Emmc> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   BluetoothLE
   EFI BluetoothLE Settings.
*/
class Bluetoothle
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "BLUETOOTHLE";

private:
    mutable QString _string = {};

public:
    QString device_address = {};
    EFIBoot::File_path::MSG::Bluetoothle::ADDRESS_TYPE address_type = {};

public:
    Bluetoothle() = default;
    Bluetoothle(const EFIBoot::File_path::MSG::Bluetoothle &bluetoothle);
    EFIBoot::File_path::MSG::Bluetoothle toEFIBootFilePath() const;

    static std::optional<Bluetoothle> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   DNS
   DNS Settings.
*/
class Dns
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "DNS";

private:
    mutable QString _string = {};

public:
    bool ipv6 = {};
    QByteArray data = {};

public:
    Dns() = default;
    Dns(const EFIBoot::File_path::MSG::Dns &dns);
    EFIBoot::File_path::MSG::Dns toEFIBootFilePath() const;

    static std::optional<Dns> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   NVDIMM NS
   This device path describes a bootable NVDIMM namespace that is defined by a namespace label.
*/
class NvdimmNs
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "NVDIMM_NS";

private:
    mutable QString _string = {};

public:
    QUuid uuid = {};

public:
    NvdimmNs() = default;
    NvdimmNs(const EFIBoot::File_path::MSG::Nvdimm_ns &nvdimm_ns);
    EFIBoot::File_path::MSG::Nvdimm_ns toEFIBootFilePath() const;

    static std::optional<NvdimmNs> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   REST Service
   REST Service Settings.
*/
class RestService
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "REST_SERVICE";

private:
    mutable QString _string = {};

public:
    EFIBoot::File_path::MSG::Rest_service::REST_SERVICE rest_service = {};
    EFIBoot::File_path::MSG::Rest_service::ACCESS_MODE access_mode = {};
    QUuid guid = {};
    QByteArray data = {};

public:
    RestService() = default;
    RestService(const EFIBoot::File_path::MSG::Rest_service &rest_service);
    EFIBoot::File_path::MSG::Rest_service toEFIBootFilePath() const;

    static std::optional<RestService> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   NVMe-oF NS
   This device path describes a bootable NVMe over Fiber namespace that is defined by a unique Namespace and Subsystem NQN identity.
*/
class NvmeOfNs
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "NVME_OF_NS";

private:
    mutable QString _string = {};

public:
    uint8_t nidt = {};
    QUuid nid = {};
    QString subsystem_nqn = {};

public:
    NvmeOfNs() = default;
    NvmeOfNs(const EFIBoot::File_path::MSG::Nvme_of_ns &nvme_of_ns);
    EFIBoot::File_path::MSG::Nvme_of_ns toEFIBootFilePath() const;

    static std::optional<NvmeOfNs> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Media
   This Device Path is used to describe the portion of a medium that is being abstracted by a boot service. For example, a Media Device Path could define which partition on a hard drive was being used.
*/

/*
   Hard Drive
   The Hard Drive Media Device Path is used to represent a partition on a hard drive.
*/
class Hd
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "HD";

private:
    mutable QString _string = {};

public:
    uint32_t partition_number = {};
    uint64_t partition_start = {};
    uint64_t partition_size = {};
    QUuid partition_signature = {};
    EFIBoot::File_path::MEDIA::Hd::PARTITION_FORMAT partition_format = {};
    EFIBoot::File_path::MEDIA::Hd::SIGNATURE_TYPE signature_type = {};

public:
    Hd() = default;
    Hd(const EFIBoot::File_path::MEDIA::Hd &hd);
    EFIBoot::File_path::MEDIA::Hd toEFIBootFilePath() const;

    static std::optional<Hd> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   CD-ROM
   The CD-ROM Media Device Path is used to define a system partition that exists on a CD-ROM.
*/
class CdRom
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "CD_ROM";

private:
    mutable QString _string = {};

public:
    uint32_t boot_entry = {};
    uint64_t partition_start = {};
    uint64_t partition_size = {};

public:
    CdRom() = default;
    CdRom(const EFIBoot::File_path::MEDIA::Cd_rom &cd_rom);
    EFIBoot::File_path::MEDIA::Cd_rom toEFIBootFilePath() const;

    static std::optional<CdRom> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   File Path
   File Path settings.
*/
class FilePath
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FILE_PATH";

private:
    mutable QString _string = {};

public:
    QString path_name = {};

public:
    FilePath() = default;
    FilePath(const EFIBoot::File_path::MEDIA::File_path &file_path);
    EFIBoot::File_path::MEDIA::File_path toEFIBootFilePath() const;

    static std::optional<FilePath> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Protocol
   The Media Protocol Device Path is used to denote the protocol that is being used in a device path at the location of the path specified.
*/
class Protocol
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "PROTOCOL";

private:
    mutable QString _string = {};

public:
    QUuid guid = {};

public:
    Protocol() = default;
    Protocol(const EFIBoot::File_path::MEDIA::Protocol &protocol);
    EFIBoot::File_path::MEDIA::Protocol toEFIBootFilePath() const;

    static std::optional<Protocol> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Firmware File
   Describes a firmware file in a firmware volume.
*/
class FirmwareFile
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FIRMWARE_FILE";

private:
    mutable QString _string = {};

public:
    QUuid name = {};

public:
    FirmwareFile() = default;
    FirmwareFile(const EFIBoot::File_path::MEDIA::Firmware_file &firmware_file);
    EFIBoot::File_path::MEDIA::Firmware_file toEFIBootFilePath() const;

    static std::optional<FirmwareFile> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Firmware Volume
   Describes a firmware volume.
*/
class FirmwareVolume
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FIRMWARE_VOLUME";

private:
    mutable QString _string = {};

public:
    QUuid name = {};

public:
    FirmwareVolume() = default;
    FirmwareVolume(const EFIBoot::File_path::MEDIA::Firmware_volume &firmware_volume);
    EFIBoot::File_path::MEDIA::Firmware_volume toEFIBootFilePath() const;

    static std::optional<FirmwareVolume> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   Relative Offset Range
   This device path node specifies a range of offsets relative to the first byte available on the device.
*/
class RelativeOffsetRange
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "RELATIVE_OFFSET_RANGE";

private:
    mutable QString _string = {};

public:
    uint32_t reserved = {};
    uint64_t starting_offset = {};
    uint64_t ending_offset = {};

public:
    RelativeOffsetRange() = default;
    RelativeOffsetRange(const EFIBoot::File_path::MEDIA::Relative_offset_range &relative_offset_range);
    EFIBoot::File_path::MEDIA::Relative_offset_range toEFIBootFilePath() const;

    static std::optional<RelativeOffsetRange> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   RAM Disk
   RAM Disk Settings.
*/
class RamDisk
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "RAM_DISK";

private:
    mutable QString _string = {};

public:
    uint64_t starting_address = {};
    uint64_t ending_address = {};
    QUuid guid = {};
    uint16_t disk_instance = {};

public:
    RamDisk() = default;
    RamDisk(const EFIBoot::File_path::MEDIA::Ram_disk &ram_disk);
    EFIBoot::File_path::MEDIA::Ram_disk toEFIBootFilePath() const;

    static std::optional<RamDisk> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   BIOS
   This Device Path is used to point to boot legacy operating systems. it is based on the BIOS Boot Specification Version 1.01.
*/

/*
   BIOS Boot Specification
   This Device Path is used to describe the booting of non-EFI-aware operating systems.
*/
class BootSpecification
{
public:
    static constexpr auto TYPE = "BIOS";
    static constexpr auto SUBTYPE = "BOOT_SPECIFICATION";

private:
    mutable QString _string = {};

public:
    uint16_t device_type = {};
    uint16_t status_flag = {};
    QString description = {};

public:
    BootSpecification() = default;
    BootSpecification(const EFIBoot::File_path::BIOS::Boot_specification &boot_specification);
    EFIBoot::File_path::BIOS::Boot_specification toEFIBootFilePath() const;

    static std::optional<BootSpecification> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

/*
   End
   Depending on the Sub-Type, this Device Path node is used to indicate the end of the Device Path instance or Device Path structure.
*/

class Vendor
{
public:
    static constexpr auto TYPE = "MULTI";
    static constexpr auto SUBTYPE = "VENDOR";

private:
    mutable QString _string = {};

public:
    QByteArray data = {};
    QUuid guid = {};
    uint8_t _type = {};

public:
    Vendor() = default;
    Vendor(const EFIBoot::File_path::HW::Vendor &vendor);
    Vendor(const EFIBoot::File_path::MSG::Vendor &vendor);
    Vendor(const EFIBoot::File_path::MEDIA::Vendor &vendor);
    EFIBoot::File_path::ANY toEFIBootFilePath() const;

    static std::optional<Vendor> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

class End
{
public:
    static constexpr auto TYPE = "END";
    static constexpr auto SUBTYPE = "MULTI";

private:
    mutable QString _string = {};

public:
    uint8_t _subtype = {};

public:
    End() = default;
    End(const EFIBoot::File_path::END::Instance &)
        : _subtype{EFIBoot::File_path::END::Instance::SUBTYPE}
    {
    }
    End(const EFIBoot::File_path::END::Entire &)
        : _subtype{EFIBoot::File_path::END::Entire::SUBTYPE}
    {
    }
    EFIBoot::File_path::ANY toEFIBootFilePath() const
    {
        switch(_subtype)
        {
        case EFIBoot::File_path::END::Instance::SUBTYPE:
            return EFIBoot::File_path::END::Instance{};

        case EFIBoot::File_path::END::Entire::SUBTYPE:
            return EFIBoot::File_path::END::Entire{};

        default:
            return {};
        }
    }

    static std::optional<End> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

class Unknown
{
public:
    static constexpr auto TYPE = "UNK";
    static constexpr auto SUBTYPE = "UNKNOWN";

private:
    mutable QString _string = {};

public:
    QByteArray data = {};
    uint8_t _type = {};
    uint8_t _subtype = {};

public:
    Unknown() = default;
    Unknown(const EFIBoot::File_path::Unknown &unknown);
    EFIBoot::File_path::Unknown toEFIBootFilePath() const;

    static std::optional<Unknown> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};

using ANY = std::variant<
    Pci,
    Pccard,
    MemoryMapped,
    Controller,
    Bmc,
    Acpi,
    Expanded,
    Adr,
    Nvdimm,
    Atapi,
    Scsi,
    FibreChannel,
    Firewire,
    Usb,
    I2o,
    Infiniband,
    MacAddress,
    Ipv4,
    Ipv6,
    Uart,
    UsbClass,
    UsbWwid,
    DeviceLogicalUnit,
    Sata,
    Iscsi,
    Vlan,
    FibreChannelEx,
    SasExtendedMessaging,
    NvmExpressNs,
    Uri,
    Ufs,
    Sd,
    Bluetooth,
    WiFi,
    Emmc,
    Bluetoothle,
    Dns,
    NvdimmNs,
    RestService,
    NvmeOfNs,
    Hd,
    CdRom,
    FilePath,
    Protocol,
    FirmwareFile,
    FirmwareVolume,
    RelativeOffsetRange,
    RamDisk,
    BootSpecification,
    Vendor,
    End,
    Unknown>;

inline std::unordered_map<QString, std::function<std::optional<ANY>(const QJsonObject &)>> JSON_readers()
{
#define reader(Type)                                                                  \
    {                                                                                 \
        QString("%1/%2").arg(Type::TYPE).arg(Type::SUBTYPE),                          \
            [](const auto &obj) -> std::optional<ANY> { return Type::fromJSON(obj); } \
    }
    return {
        reader(Pci),
        reader(Pccard),
        reader(MemoryMapped),
        reader(Controller),
        reader(Bmc),
        reader(Acpi),
        // Old ACPI subtype:
        {"ACPI/HID", [](const auto &obj) -> std::optional<ANY>
            { return Acpi::fromJSON(obj); }},
        reader(Expanded),
        reader(Adr),
        reader(Nvdimm),
        reader(Atapi),
        reader(Scsi),
        reader(FibreChannel),
        reader(Firewire),
        reader(Usb),
        reader(I2o),
        reader(Infiniband),
        reader(MacAddress),
        reader(Ipv4),
        reader(Ipv6),
        reader(Uart),
        reader(UsbClass),
        reader(UsbWwid),
        reader(DeviceLogicalUnit),
        reader(Sata),
        reader(Iscsi),
        reader(Vlan),
        reader(FibreChannelEx),
        reader(SasExtendedMessaging),
        reader(NvmExpressNs),
        reader(Uri),
        reader(Ufs),
        reader(Sd),
        reader(Bluetooth),
        reader(WiFi),
        reader(Emmc),
        reader(Bluetoothle),
        reader(Dns),
        reader(NvdimmNs),
        reader(RestService),
        reader(NvmeOfNs),
        reader(Hd),
        reader(CdRom),
        reader(FilePath),
        // Old FilePath subtype:
        {"MEDIA/FILE", [](const auto &obj) -> std::optional<ANY>
            { return FilePath::fromJSON(obj); }},
        reader(Protocol),
        reader(FirmwareFile),
        reader(FirmwareVolume),
        reader(RelativeOffsetRange),
        reader(RamDisk),
        reader(BootSpecification),
        // Old BootSpecification subtype:
        {"BIOS/BIOS_BOOT_SPECIFICATION", [](const auto &obj) -> std::optional<ANY>
            { return BootSpecification::fromJSON(obj); }},
        reader(Vendor),
        reader(End),
        reader(Unknown),
    };
#undef reader
}

} // namespace FilePath

Q_DECLARE_METATYPE(const FilePath::ANY *)

class BootEntry
{
private:
    mutable QString device_path_str = {};

public:
    enum class OptionalDataFormat : uint8_t
    {
        Base64 = 0,
        Utf16 = 1,
        Utf8 = 2,
        Hex = 3,
    };

    QVector<FilePath::ANY> device_path = {};
    QString description = "New entry";
    QString error = {};
    QString optional_data = {};
    EFIBoot::Load_option_attribute attributes = EFIBoot::Load_option_attribute::EMPTY;
    uint32_t efi_attributes = EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS;
    uint32_t crc32 = 0;
    uint16_t index = 0;
    OptionalDataFormat optional_data_format = OptionalDataFormat::Base64;

    bool is_current_boot = false;
    bool is_next_boot = false;
    bool is_error = false;

public:
    static BootEntry fromEFIBootLoadOption(const EFIBoot::Load_option &load_option);
    static BootEntry fromError(const QString &error);
    EFIBoot::Load_option toEFIBootLoadOption() const;

    static std::optional<BootEntry> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString formatDevicePath(bool refresh = true) const;
    QString getTitle() const;

    bool changeOptionalDataFormat(OptionalDataFormat format, bool test = false);

private:
    QByteArray getRawOptionalData() const;
};

Q_DECLARE_METATYPE(const BootEntry *)
