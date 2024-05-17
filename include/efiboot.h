// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "compat.h"

namespace EFIBoot
{

extern "C"
{
#include "efivar-lite/efivar-lite.h"
#include "efivar-lite/load-option.h"
}

inline bool operator==(const efi_guid_t &first, const efi_guid_t &second)
{
    return efi_guid_cmp(&first, &second) == 0;
}

inline bool operator!=(const efi_guid_t &first, const efi_guid_t &second)
{
    return efi_guid_cmp(&first, &second) != 0;
}

using Raw_data = std::vector<uint8_t>;

template <class Type = Raw_data>
std::optional<Type> deserialize(const void *data, size_t data_size);

namespace File_path
{

/*
   Hardware
   This Device Path defines how a device is attached to the resource domain of a system, where resource domain is simply the shared memory, memory mapped I/O, and I/O space of the system.
*/
namespace HW
{

/*
   PCI
   The Device Path for PCI defines the path to the PCI configuration space address for a PCI device.
*/
struct Pci
{
    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_PCI;

    uint8_t function = {};
    uint8_t device = {};
};

/*
   PCCARD
   PCCARD Settings.
*/
struct Pccard
{
    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_PCCARD;

    uint8_t function_number = {};
};

/*
   Memory Mapped
   Memory Mapped Settings.
*/
struct Memory_mapped
{
    enum class MEMORY_TYPE : uint32_t
    {
        RESERVED = 0x0,
        LOADER_CODE = 0x1,
        LOADER_DATA = 0x2,
        BOOT_SERVICES_CODE = 0x3,
        BOOT_SERVICES_DATA = 0x4,
        RUNTIME_SERVICES_CODE = 0x5,
        RUNTIME_SERVICES_DATA = 0x6,
        CONVENTIONAL = 0x7,
        UNUSABLE = 0x8,
        ACPI_RECLAIM = 0x9,
        ACPI_MEMORY_NVS = 0xa,
        MEMORY_MAPPED_IO = 0xb,
        MEMORY_MAPPD_IO_PORT_SPACE = 0xc,
        PAL_CODE = 0xd,
        PERSISTENT = 0xe,
        UNACCEPTED = 0xf,
    };

    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_MEMORY_MAPPED;

    MEMORY_TYPE memory_type = {};
    uint64_t start_address = {};
    uint64_t end_address = {};
};

/*
   Vendor-Defined Hardware
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
struct Vendor
{
    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_VENDOR;

    std::array<uint8_t, 16> guid = {};
    Raw_data data = {};
};

/*
   Controller
   Controller settings.
*/
struct Controller
{
    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_CONTROLLER;

    uint32_t controller_number = {};
};

/*
   BMC
   The Device Path for a Baseboard Management Controller (BMC) host interface.
*/
struct Bmc
{
    enum class INTERFACE_TYPE : uint8_t
    {
        UNKNOWN = 0x0,
        KCS = 0x1,
        SMIC = 0x2,
        BT = 0x3,
    };

    static const uint8_t TYPE = EFIDP_TYPE_HW;
    static const uint8_t SUBTYPE = EFIDP_HW_BMC;

    INTERFACE_TYPE interface_type = {};
    uint64_t base_address = {};
};

} // namespace HW

/*
   ACPI
   This Device Path is used to describe devices whose enumeration is not described in an industry-standard fashion. These devices must be described using ACPI AML in the ACPI name space; this Device Path is a linkage to the ACPI name space.
*/
namespace ACPI
{

/*
   ACPI
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
struct Acpi
{
    static const uint8_t TYPE = EFIDP_TYPE_ACPI;
    static const uint8_t SUBTYPE = EFIDP_ACPI_ACPI;

    uint32_t hid = {};
    uint32_t uid = {};
};

/*
   Expanded
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
struct Expanded
{
    static const uint8_t TYPE = EFIDP_TYPE_ACPI;
    static const uint8_t SUBTYPE = EFIDP_ACPI_EXPANDED;

    uint32_t hid = {};
    uint32_t uid = {};
    uint32_t cid = {};
    std::string hidstr = {};
    std::string uidstr = {};
    std::string cidstr = {};
};

/*
   ADR
   The ADR device path is used to contain video output device attributes to support the Graphics Output Protocol.
*/
struct Adr
{
    static const uint8_t TYPE = EFIDP_TYPE_ACPI;
    static const uint8_t SUBTYPE = EFIDP_ACPI_ADR;

    uint32_t adr = {};
    Raw_data additional_adr = {};
};

/*
   NVDIMM
   This device path describes an NVDIMM device using the ACPI 6.0 specification defined NFIT Device Handle as the identifier.
*/
struct Nvdimm
{
    static const uint8_t TYPE = EFIDP_TYPE_ACPI;
    static const uint8_t SUBTYPE = EFIDP_ACPI_NVDIMM;

    uint32_t nfit_device_handle = {};
};

} // namespace ACPI

/*
   Messaging
   This Device Path is used to describe the connection of devices outside the resource domain of the system. This Device Path can describe physical messaging information such as a SCSI ID, or abstract information such as networking protocol IP addresses.
*/
namespace MSG
{

/*
   ATAPI
   ATAPI Settings.
*/
struct Atapi
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_ATAPI;

    bool primary = {};
    bool slave = {};
    uint16_t lun = {};
};

/*
   SCSI
   SCSI Settings.
*/
struct Scsi
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_SCSI;

    uint16_t pun = {};
    uint16_t lun = {};
};

/*
   Fibre Channel
   Fibre Channel Settings
*/
struct Fibre_channel
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_FIBRE_CHANNEL;

    uint32_t reserved = {};
    uint64_t world_wide_name = {};
    uint64_t lun = {};
};

/*
   Firewire
   Firewire Settings.
*/
struct Firewire
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_FIREWIRE;

    uint32_t reserved = {};
    uint64_t guid = {};
};

/*
   USB
   USB settings.
*/
struct Usb
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_USB;

    uint8_t parent_port_number = {};
    uint8_t interface_number = {};
};

/*
   I2O
   I2O Settings
*/
struct I2o
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_I2O;

    uint32_t tid = {};
};

/*
   InfiniBand
   InfiniBand Settings.
*/
struct Infiniband
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_INFINIBAND;

    uint32_t resource_flags = {};
    std::array<uint8_t, 16> port_gid = {};
    uint64_t ioc_guid_service_id = {};
    uint64_t target_port_id = {};
    uint64_t device_id = {};
};

/*
   Vendor-Defined Messaging
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
struct Vendor
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_VENDOR;

    std::array<uint8_t, 16> guid = {};
    Raw_data data = {};
};

/*
   MAC Address
   MAC settings.
*/
struct Mac_address
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_MAC_ADDRESS;

    std::array<uint8_t, 32> address = {};
    uint8_t if_type = {};
};

/*
   IPv4
   IPv4 settings.
*/
struct Ipv4
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_IPV4;

    std::array<uint8_t, 4> local_ip_address = {};
    std::array<uint8_t, 4> remote_ip_address = {};
    uint16_t local_port = {};
    uint16_t remote_port = {};
    uint16_t protocol = {};
    bool static_ip_address = {};
    std::array<uint8_t, 4> gateway_ip_address = {};
    std::array<uint8_t, 4> subnet_mask = {};
};

/*
   IPv6
   IPv6 settings.
*/
struct Ipv6
{
    enum class IP_ADDRESS_ORIGIN : uint8_t
    {
        STATIC = 0x0,
        STATELESS = 0x1,
        STATEFUL = 0x2,
    };

    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_IPV6;

    std::array<uint8_t, 16> local_ip_address = {};
    std::array<uint8_t, 16> remote_ip_address = {};
    uint16_t local_port = {};
    uint16_t remote_port = {};
    uint16_t protocol = {};
    IP_ADDRESS_ORIGIN ip_address_origin = {};
    uint8_t prefix_length = {};
    std::array<uint8_t, 16> gateway_ip_address = {};
};

/*
   UART
   UART Settings.
*/
struct Uart
{
    enum class PARITY : uint8_t
    {
        DEFAULT = 0x0,
        NO = 0x1,
        EVEN = 0x2,
        ODD = 0x3,
        MARK = 0x4,
        SPACE = 0x5,
    };

    enum class STOP_BITS : uint8_t
    {
        DEFAULT = 0x0,
        ONE = 0x1,
        ONE_AND_HALF = 0x2,
        TWO = 0x3,
    };

    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_UART;

    uint32_t reserved = {};
    uint64_t baud_rate = {};
    uint8_t data_bits = {};
    PARITY parity = {};
    STOP_BITS stop_bits = {};
};

/*
   USB Class
   USB Class Settings.
*/
struct Usb_class
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_USB_CLASS;

    uint16_t vendor_id = {};
    uint16_t product_id = {};
    uint8_t device_class = {};
    uint8_t device_subclass = {};
    uint8_t device_protocol = {};
};

/*
   USB WWID
   This device path describes a USB device using its serial number.
*/
struct Usb_wwid
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_USB_WWID;

    uint16_t interface_number = {};
    uint16_t device_vendor_id = {};
    uint16_t device_product_id = {};
    std::u16string serial_number = {};
};

/*
   Device Logical Unit
   Device Logical Unit Settings.
*/
struct Device_logical_unit
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_DEVICE_LOGICAL_UNIT;

    uint8_t lun = {};
};

/*
   SATA
   SATA settings.
*/
struct Sata
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_SATA;

    uint16_t hba_port_number = {};
    uint16_t port_multiplier_port_number = {};
    uint16_t lun = {};
};

/*
   iSCSI
   iSCSI Settings.
*/
struct Iscsi
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_ISCSI;

    uint16_t protocol = {};
    uint16_t options = {};
    uint64_t lun = {};
    uint16_t target_portal_group = {};
    std::string target_name = {};
};

/*
   VLAN
   VLAN Settings.
*/
struct Vlan
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_VLAN;

    uint16_t vlan_id = {};
};

/*
   Fibre Channel Ex
   The Fibre Channel Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
struct Fibre_channel_ex
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_FIBRE_CHANNEL_EX;

    uint32_t reserved = {};
    uint64_t world_wide_name = {};
    uint64_t lun = {};
};

/*
   SAS Extended Messaging
   The SAS Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
struct Sas_extended_messaging
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_SAS_EXTENDED_MESSAGING;

    uint64_t sas_address = {};
    uint64_t lun = {};
    uint16_t device_and_topology_info = {};
    uint16_t relative_target_port = {};
};

/*
   NVM Express NS
   NVM Express Namespace Settings.
*/
struct Nvm_express_ns
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_NVM_EXPRESS_NS;

    uint32_t namespace_identifier = {};
    uint64_t ieee_extended_unique_identifier = {};
};

/*
   URI
   Refer to RFC 3986 for details on the URI contents.
*/
struct Uri
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_URI;

    Raw_data uri = {};
};

/*
   UFS
   UFS Settings.
*/
struct Ufs
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_UFS;

    uint8_t pun = {};
    uint8_t lun = {};
};

/*
   SD
   SD Settings.
*/
struct Sd
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_SD;

    uint8_t slot_number = {};
};

/*
   Bluetooth
   EFI Bluetooth Settings.
*/
struct Bluetooth
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_BLUETOOTH;

    std::array<uint8_t, 6> device_address = {};
};

/*
   Wi-Fi
   Wi-Fi Settings.
*/
struct Wi_fi
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_WI_FI;

    std::string ssid = {};
};

/*
   eMMC
   Embedded Multi-Media Card Settings.
*/
struct Emmc
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_EMMC;

    uint8_t slot_number = {};
};

/*
   BluetoothLE
   EFI BluetoothLE Settings.
*/
struct Bluetoothle
{
    enum class ADDRESS_TYPE : uint8_t
    {
        PUBLIC = 0x0,
        RANDOM = 0x1,
    };

    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_BLUETOOTHLE;

    std::array<uint8_t, 6> device_address = {};
    ADDRESS_TYPE address_type = {};
};

/*
   DNS
   DNS Settings.
*/
struct Dns
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_DNS;

    bool ipv6 = {};
    Raw_data data = {};
};

/*
   NVDIMM NS
   This device path describes a bootable NVDIMM namespace that is defined by a namespace label.
*/
struct Nvdimm_ns
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_NVDIMM_NS;

    std::array<uint8_t, 16> uuid = {};
};

/*
   REST Service
   REST Service Settings.
*/
struct Rest_service
{
    enum class REST_SERVICE : uint8_t
    {
        REDFISH = 0x1,
        ODATA = 0x2,
        VENDOR = 0xff,
    };

    enum class ACCESS_MODE : uint8_t
    {
        IN_BAND = 0x1,
        OUT_OF_BAND = 0x2,
    };

    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_REST_SERVICE;

    REST_SERVICE rest_service = {};
    ACCESS_MODE access_mode = {};
    std::array<uint8_t, 16> guid = {};
    Raw_data data = {};
};

/*
   NVMe-oF NS
   This device path describes a bootable NVMe over Fiber namespace that is defined by a unique Namespace and Subsystem NQN identity.
*/
struct Nvme_of_ns
{
    static const uint8_t TYPE = EFIDP_TYPE_MSG;
    static const uint8_t SUBTYPE = EFIDP_MSG_NVME_OF_NS;

    uint8_t nidt = {};
    std::array<uint8_t, 16> nid = {};
    std::string subsystem_nqn = {};
};

} // namespace MSG

/*
   Media
   This Device Path is used to describe the portion of a medium that is being abstracted by a boot service. For example, a Media Device Path could define which partition on a hard drive was being used.
*/
namespace MEDIA
{

/*
   Hard Drive
   The Hard Drive Media Device Path is used to represent a partition on a hard drive.
*/
struct Hd
{
    enum class PARTITION_FORMAT : uint8_t
    {
        MBR = 0x1,
        GUID = 0x2,
    };

    enum class SIGNATURE_TYPE : uint8_t
    {
        NONE = 0x0,
        MBR = 0x1,
        GUID = 0x2,
    };

    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_HD;

    uint32_t partition_number = {};
    uint64_t partition_start = {};
    uint64_t partition_size = {};
    std::array<uint8_t, 16> partition_signature = {};
    PARTITION_FORMAT partition_format = {};
    SIGNATURE_TYPE signature_type = {};
};

/*
   CD-ROM
   The CD-ROM Media Device Path is used to define a system partition that exists on a CD-ROM.
*/
struct Cd_rom
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_CD_ROM;

    uint32_t boot_entry = {};
    uint64_t partition_start = {};
    uint64_t partition_size = {};
};

/*
   Vendor-Defined Media
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
struct Vendor
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_VENDOR;

    std::array<uint8_t, 16> guid = {};
    Raw_data data = {};
};

/*
   File Path
   File Path settings.
*/
struct File_path
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_FILE_PATH;

    std::u16string path_name = {};
};

/*
   Protocol
   The Media Protocol Device Path is used to denote the protocol that is being used in a device path at the location of the path specified.
*/
struct Protocol
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_PROTOCOL;

    std::array<uint8_t, 16> guid = {};
};

/*
   Firmware File
   Describes a firmware file in a firmware volume.
*/
struct Firmware_file
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_FIRMWARE_FILE;

    std::array<uint8_t, 16> name = {};
};

/*
   Firmware Volume
   Describes a firmware volume.
*/
struct Firmware_volume
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_FIRMWARE_VOLUME;

    std::array<uint8_t, 16> name = {};
};

/*
   Relative Offset Range
   This device path node specifies a range of offsets relative to the first byte available on the device.
*/
struct Relative_offset_range
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_RELATIVE_OFFSET_RANGE;

    uint32_t reserved = {};
    uint64_t starting_offset = {};
    uint64_t ending_offset = {};
};

/*
   RAM Disk
   RAM Disk Settings.
*/
struct Ram_disk
{
    static const uint8_t TYPE = EFIDP_TYPE_MEDIA;
    static const uint8_t SUBTYPE = EFIDP_MEDIA_RAM_DISK;

    uint64_t starting_address = {};
    uint64_t ending_address = {};
    std::array<uint8_t, 16> guid = {};
    uint16_t disk_instance = {};
};

} // namespace MEDIA

/*
   BIOS
   This Device Path is used to point to boot legacy operating systems. it is based on the BIOS Boot Specification Version 1.01.
*/
namespace BIOS
{

/*
   BIOS Boot Specification
   This Device Path is used to describe the booting of non-EFI-aware operating systems.
*/
struct Boot_specification
{
    static const uint8_t TYPE = EFIDP_TYPE_BIOS;
    static const uint8_t SUBTYPE = EFIDP_BIOS_BOOT_SPECIFICATION;

    uint16_t device_type = {};
    uint16_t status_flag = {};
    std::string description = {};
};

} // namespace BIOS

/*
   End
   Depending on the Sub-Type, this Device Path node is used to indicate the end of the Device Path instance or Device Path structure.
*/
namespace END
{

/*
   End This Instance
   This type of node terminates one Device Path instance and denotes the start of another. This is only required when an environment variable represents multiple devices.
*/
struct Instance
{
    static const uint8_t TYPE = EFIDP_TYPE_END;
    static const uint8_t SUBTYPE = EFIDP_END_INSTANCE;
};

/*
   End Entire
   This type of node terminates an entire Device Path. Software searches for this sub-type to find the end of a Device Path. All Device Paths must end with this sub-type.
*/
struct Entire
{
    static const uint8_t TYPE = EFIDP_TYPE_END;
    static const uint8_t SUBTYPE = EFIDP_END_ENTIRE;
};

} // namespace END

struct Unknown
{
    Raw_data data = {};
    uint8_t TYPE = {};
    uint8_t SUBTYPE = {};
};

using ANY = std::variant<
    HW::Pci,
    HW::Pccard,
    HW::Memory_mapped,
    HW::Vendor,
    HW::Controller,
    HW::Bmc,
    ACPI::Acpi,
    ACPI::Expanded,
    ACPI::Adr,
    ACPI::Nvdimm,
    MSG::Atapi,
    MSG::Scsi,
    MSG::Fibre_channel,
    MSG::Firewire,
    MSG::Usb,
    MSG::I2o,
    MSG::Infiniband,
    MSG::Vendor,
    MSG::Mac_address,
    MSG::Ipv4,
    MSG::Ipv6,
    MSG::Uart,
    MSG::Usb_class,
    MSG::Usb_wwid,
    MSG::Device_logical_unit,
    MSG::Sata,
    MSG::Iscsi,
    MSG::Vlan,
    MSG::Fibre_channel_ex,
    MSG::Sas_extended_messaging,
    MSG::Nvm_express_ns,
    MSG::Uri,
    MSG::Ufs,
    MSG::Sd,
    MSG::Bluetooth,
    MSG::Wi_fi,
    MSG::Emmc,
    MSG::Bluetoothle,
    MSG::Dns,
    MSG::Nvdimm_ns,
    MSG::Rest_service,
    MSG::Nvme_of_ns,
    MEDIA::Hd,
    MEDIA::Cd_rom,
    MEDIA::Vendor,
    MEDIA::File_path,
    MEDIA::Protocol,
    MEDIA::Firmware_file,
    MEDIA::Firmware_volume,
    MEDIA::Relative_offset_range,
    MEDIA::Ram_disk,
    BIOS::Boot_specification,
    END::Instance,
    END::Entire,
    Unknown>;

} // namespace File_path

enum class Load_option_attribute : uint32_t
{
    EMPTY = 0x00000000,

    ACTIVE = 0x00000001,
    FORCE_RECONNECT = 0x00000002,
    HIDDEN = 0x00000008,

    CATEGORY_MASK = 0x00001F00,
    CATEGORY_BOOT = 0x00000000,
    CATEGORY_APP = 0x00000100,
};

inline Load_option_attribute operator|(Load_option_attribute a, Load_option_attribute b)
{
    return static_cast<Load_option_attribute>(static_cast<std::underlying_type_t<Load_option_attribute>>(a) | static_cast<std::underlying_type_t<Load_option_attribute>>(b));
}

inline Load_option_attribute operator&(Load_option_attribute a, Load_option_attribute b)
{
    return static_cast<Load_option_attribute>(static_cast<std::underlying_type_t<Load_option_attribute>>(a) & static_cast<std::underlying_type_t<Load_option_attribute>>(b));
}

struct Load_option
{
    std::u16string description = u"";
    std::vector<File_path::ANY> device_path = {};
    Raw_data optional_data = {};
    Load_option_attribute attributes = Load_option_attribute::EMPTY;
};

using Progress_fn = std::function<void(size_t, size_t)>;

template <class Type = Raw_data>
using Variable = std::tuple<Type, uint32_t>;

std::optional<tstring> init();

template <class Type = Raw_data>
std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size);

template <class Type = Raw_data, class Size_fn, class Advance_fn>
std::optional<std::vector<Type>> deserialize_list_ex(const void *data, size_t data_size, const Size_fn &get_element_size, const Advance_fn &get_next_element);

template <class Type = Raw_data>
size_t serialize(Raw_data &output, const Type &value);

template <class Type = Raw_data>
size_t serialize_list(Raw_data &output, const std::vector<Type> &value);

template <class Filter_fn>
std::unordered_map<tstring, efi_guid_t> get_variables(const Filter_fn &filter, const Progress_fn &progress);

template <class Filter_fn>
std::unordered_map<tstring, efi_guid_t> get_variables(const Filter_fn &filter);
std::unordered_map<tstring, efi_guid_t> get_variables();

template <class Type = Raw_data>
std::optional<Variable<Type>> get_variable(const efi_guid_t &guid, const tstring &name);

template <class Type = Raw_data>
std::optional<Variable<std::vector<Type>>> get_list_variable(const efi_guid_t &guid, const tstring &name);

template <class Type = Raw_data, class Size_fn, class Advance_fn>
std::optional<Variable<std::vector<Type>>> get_list_variable_ex(const efi_guid_t &guid, const tstring &name, const Size_fn &get_element_size, const Advance_fn &get_next_element);

template <class Type = Raw_data>
bool set_variable(const efi_guid_t &guid, const tstring &name, const Variable<Type> &variable, mode_t mode);

template <class Type = Raw_data>
bool set_list_variable(const efi_guid_t &guid, const tstring &name, const Variable<std::vector<Type>> &variable, mode_t mode);

inline std::optional<tstring> init()
{
    if(!efi_variables_supported())
        return {_T("UEFI variables not supported on this machine.")};

    return std::nullopt;
}

template <class Type>
inline std::optional<Type> deserialize(const void *data, size_t data_size)
{
    if(data_size != sizeof(Type))
        return std::nullopt;

    return {*static_cast<const Type *>(data)};
}

template <class Type>
inline size_t serialize(Raw_data &output, const Type &value)
{
    size_t pos = output.size();
    output.resize(pos + sizeof(value));
    memcpy(&output[pos], &value, sizeof(value));
    return sizeof(value);
}

template <>
inline std::optional<Raw_data> deserialize<Raw_data>(const void *data, size_t data_size)
{
    return {Raw_data{static_cast<const uint8_t *>(data), static_cast<const uint8_t *>(advance_bytes(data, data_size))}};
}

template <>
inline size_t serialize(Raw_data &output, const Raw_data &data)
{
    output.insert(std::end(output), std::begin(data), std::end(data));
    return data.size();
}

template <>
inline std::optional<std::string> deserialize<std::string>(const void *data, size_t data_size)
{
    if(data_size % sizeof(std::string::value_type))
        return std::nullopt;

    return {std::string{static_cast<const std::string::value_type *>(data), data_size / sizeof(std::string::value_type)}};
}

template <>
inline size_t serialize(Raw_data &output, const std::string &value)
{
    size_t bytes = (value.size() + 1) * sizeof(std::string::value_type);
    size_t pos = output.size();
    output.resize(pos + bytes);
    memcpy(&output[pos], value.c_str(), bytes);
    return bytes;
}

template <>
inline std::optional<std::wstring> deserialize<std::wstring>(const void *data, size_t data_size)
{
    if(data_size % sizeof(std::wstring::value_type))
        return std::nullopt;

    return {std::wstring{static_cast<const std::wstring::value_type *>(data), data_size / sizeof(std::wstring::value_type)}};
}

template <>
inline size_t serialize(Raw_data &output, const std::wstring &value)
{
    size_t bytes = (value.size() + 1) * sizeof(std::wstring::value_type);
    size_t pos = output.size();
    output.resize(pos + bytes);
    memcpy(&output[pos], value.c_str(), bytes);
    return bytes;
}

template <>
inline std::optional<std::u16string> deserialize(const void *data, size_t data_size)
{
    if(data_size % sizeof(std::u16string::value_type))
        return std::nullopt;

    return {std::u16string{static_cast<const std::u16string::value_type *>(data), data_size / sizeof(std::u16string::value_type)}};
}

template <>
inline size_t serialize(Raw_data &output, const std::u16string &value)
{
    size_t bytes = (value.size() + 1) * sizeof(std::u16string::value_type);
    size_t pos = output.size();
    output.resize(pos + bytes);
    memcpy(&output[pos], value.c_str(), bytes);
    return bytes;
}

template <class Type, class Size_fn, class Advance_fn>
inline std::optional<std::vector<Type>> deserialize_list_ex(const void *data, size_t data_size, const Size_fn &get_element_size, const Advance_fn &get_next_element)
{
    std::vector<Type> values;
    const void *data_end = advance_bytes(data, data_size);
    while(data && data < data_end)
    {
        auto element_size = get_element_size(data);
        auto value = deserialize<Type>(data, element_size);
        if(!value)
            return std::nullopt;

        values.push_back(*value);
        data = get_next_element(data, data_size);
        auto bytes_left = static_cast<const uint8_t *>(data_end) - static_cast<const uint8_t *>(data);
        data_size = static_cast<size_t>(bytes_left);
    }

    if(data != data_end)
        return std::nullopt;

    return {values};
}

template <class Type>
inline std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size)
{
    return deserialize_list_ex<Type>(
        data, data_size,
        [](const void *)
        {
            return sizeof(Type);
        },
        [](const void *ptr, size_t)
        {
            return advance_bytes(ptr, sizeof(const Type));
        });
}

template <class Type>
size_t serialize_list(Raw_data &output, const std::vector<Type> &value)
{
    size_t bytes = 0;
    for(const auto &item: value)
        bytes += serialize(output, item);

    return bytes;
}

// Hardware

template <>
inline std::optional<File_path::HW::Pci> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_pci *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Pci::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Pci::SUBTYPE)
        return std::nullopt;

    File_path::HW::Pci value{};
    value.function = dp->function;
    value.device = dp->device;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Pci &pci)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Pci::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Pci::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, pci.function);
    bytes += serialize(output, pci.device);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::HW::Pccard> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_pccard *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Pccard::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Pccard::SUBTYPE)
        return std::nullopt;

    File_path::HW::Pccard value{};
    value.function_number = dp->function_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Pccard &pccard)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Pccard::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Pccard::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, pccard.function_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::HW::Memory_mapped> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_memory_mapped *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Memory_mapped::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Memory_mapped::SUBTYPE)
        return std::nullopt;

    File_path::HW::Memory_mapped value{};
    value.memory_type = static_cast<File_path::HW::Memory_mapped::MEMORY_TYPE>(dp->memory_type);
    value.start_address = dp->start_address;
    value.end_address = dp->end_address;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Memory_mapped &memory_mapped)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Memory_mapped::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Memory_mapped::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, memory_mapped.memory_type);
    bytes += serialize(output, memory_mapped.start_address);
    bytes += serialize(output, memory_mapped.end_address);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::HW::Vendor> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_vendor *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Vendor::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Vendor::SUBTYPE)
        return std::nullopt;

    File_path::HW::Vendor value{};
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    {
        static_assert(sizeof(*value.data.data()) == sizeof(dp->data[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->data[0])>(advance_bytes(data, data_size)) - &dp->data[0]);
        value.data.resize(length);
        memcpy(value.data.data(), dp->data, length * sizeof(dp->data[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Vendor &vendor)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Vendor::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Vendor::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, vendor.guid);
    bytes += serialize(output, vendor.data);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::HW::Controller> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_controller *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Controller::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Controller::SUBTYPE)
        return std::nullopt;

    File_path::HW::Controller value{};
    value.controller_number = dp->controller_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Controller &controller)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Controller::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Controller::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, controller.controller_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::HW::Bmc> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_hw_bmc *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::HW::Bmc::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::HW::Bmc::SUBTYPE)
        return std::nullopt;

    File_path::HW::Bmc value{};
    value.interface_type = static_cast<File_path::HW::Bmc::INTERFACE_TYPE>(dp->interface_type);
    value.base_address = dp->base_address;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::HW::Bmc &bmc)
{
    size_t bytes = 0;
    uint8_t type = File_path::HW::Bmc::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::HW::Bmc::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, bmc.interface_type);
    bytes += serialize(output, bmc.base_address);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

// ACPI

template <>
inline std::optional<File_path::ACPI::Acpi> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_acpi_acpi *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::ACPI::Acpi::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::ACPI::Acpi::SUBTYPE)
        return std::nullopt;

    File_path::ACPI::Acpi value{};
    value.hid = dp->hid;
    value.uid = dp->uid;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::ACPI::Acpi &acpi)
{
    size_t bytes = 0;
    uint8_t type = File_path::ACPI::Acpi::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::ACPI::Acpi::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, acpi.hid);
    bytes += serialize(output, acpi.uid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::ACPI::Expanded> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_acpi_expanded *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::ACPI::Expanded::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::ACPI::Expanded::SUBTYPE)
        return std::nullopt;

    File_path::ACPI::Expanded value{};
    value.hid = dp->hid;
    value.uid = dp->uid;
    value.cid = dp->cid;
    value.hidstr = reinterpret_cast<const decltype(value.hidstr)::value_type *>(dp->hidstr);
    size_t offset = value.hidstr.size() + 1;
    value.uidstr = reinterpret_cast<const decltype(value.uidstr)::value_type *>(dp->hidstr + offset);
    offset += value.uidstr.size() + 1;
    value.cidstr = reinterpret_cast<const decltype(value.cidstr)::value_type *>(dp->hidstr + offset);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::ACPI::Expanded &expanded)
{
    size_t bytes = 0;
    uint8_t type = File_path::ACPI::Expanded::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::ACPI::Expanded::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, expanded.hid);
    bytes += serialize(output, expanded.uid);
    bytes += serialize(output, expanded.cid);
    bytes += serialize(output, expanded.hidstr);
    bytes += serialize(output, expanded.uidstr);
    bytes += serialize(output, expanded.cidstr);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::ACPI::Adr> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_acpi_adr *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::ACPI::Adr::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::ACPI::Adr::SUBTYPE)
        return std::nullopt;

    File_path::ACPI::Adr value{};
    value.adr = dp->adr;
    {
        static_assert(sizeof(*value.additional_adr.data()) == sizeof(dp->additional_adr[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->additional_adr[0])>(advance_bytes(data, data_size)) - &dp->additional_adr[0]);
        value.additional_adr.resize(length);
        memcpy(value.additional_adr.data(), dp->additional_adr, length * sizeof(dp->additional_adr[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::ACPI::Adr &adr)
{
    size_t bytes = 0;
    uint8_t type = File_path::ACPI::Adr::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::ACPI::Adr::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, adr.adr);
    bytes += serialize(output, adr.additional_adr);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::ACPI::Nvdimm> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_acpi_nvdimm *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::ACPI::Nvdimm::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::ACPI::Nvdimm::SUBTYPE)
        return std::nullopt;

    File_path::ACPI::Nvdimm value{};
    value.nfit_device_handle = dp->nfit_device_handle;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::ACPI::Nvdimm &nvdimm)
{
    size_t bytes = 0;
    uint8_t type = File_path::ACPI::Nvdimm::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::ACPI::Nvdimm::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, nvdimm.nfit_device_handle);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

// Messaging

template <>
inline std::optional<File_path::MSG::Atapi> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_atapi *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Atapi::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Atapi::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Atapi value{};
    value.primary = dp->primary;
    value.slave = dp->slave;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Atapi &atapi)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Atapi::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Atapi::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, atapi.primary);
    bytes += serialize(output, atapi.slave);
    bytes += serialize(output, atapi.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Scsi> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_scsi *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Scsi::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Scsi::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Scsi value{};
    value.pun = dp->pun;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Scsi &scsi)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Scsi::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Scsi::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, scsi.pun);
    bytes += serialize(output, scsi.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Fibre_channel> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_fibre_channel *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Fibre_channel::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Fibre_channel::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Fibre_channel value{};
    value.reserved = dp->reserved;
    value.world_wide_name = dp->world_wide_name;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Fibre_channel &fibre_channel)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Fibre_channel::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Fibre_channel::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, fibre_channel.reserved);
    bytes += serialize(output, fibre_channel.world_wide_name);
    bytes += serialize(output, fibre_channel.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Firewire> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_firewire *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Firewire::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Firewire::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Firewire value{};
    value.reserved = dp->reserved;
    value.guid = dp->guid;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Firewire &firewire)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Firewire::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Firewire::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, firewire.reserved);
    bytes += serialize(output, firewire.guid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Usb> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_usb *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Usb::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Usb::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Usb value{};
    value.parent_port_number = dp->parent_port_number;
    value.interface_number = dp->interface_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Usb &usb)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Usb::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Usb::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, usb.parent_port_number);
    bytes += serialize(output, usb.interface_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::I2o> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_i2o *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::I2o::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::I2o::SUBTYPE)
        return std::nullopt;

    File_path::MSG::I2o value{};
    value.tid = dp->tid;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::I2o &i2o)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::I2o::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::I2o::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, i2o.tid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Infiniband> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_infiniband *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Infiniband::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Infiniband::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Infiniband value{};
    value.resource_flags = dp->resource_flags;
    std::copy(std::begin(dp->port_gid), std::end(dp->port_gid), std::begin(value.port_gid));
    value.ioc_guid_service_id = dp->ioc_guid_service_id;
    value.target_port_id = dp->target_port_id;
    value.device_id = dp->device_id;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Infiniband &infiniband)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Infiniband::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Infiniband::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, infiniband.resource_flags);
    bytes += serialize(output, infiniband.port_gid);
    bytes += serialize(output, infiniband.ioc_guid_service_id);
    bytes += serialize(output, infiniband.target_port_id);
    bytes += serialize(output, infiniband.device_id);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Vendor> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_vendor *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Vendor::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Vendor::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Vendor value{};
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    {
        static_assert(sizeof(*value.data.data()) == sizeof(dp->data[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->data[0])>(advance_bytes(data, data_size)) - &dp->data[0]);
        value.data.resize(length);
        memcpy(value.data.data(), dp->data, length * sizeof(dp->data[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Vendor &vendor)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Vendor::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Vendor::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, vendor.guid);
    bytes += serialize(output, vendor.data);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Mac_address> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_mac_address *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Mac_address::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Mac_address::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Mac_address value{};
    std::copy(std::begin(dp->address), std::end(dp->address), std::begin(value.address));
    value.if_type = dp->if_type;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Mac_address &mac_address)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Mac_address::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Mac_address::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, mac_address.address);
    bytes += serialize(output, mac_address.if_type);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Ipv4> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_ipv4 *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Ipv4::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Ipv4::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Ipv4 value{};
    std::copy(std::begin(dp->local_ip_address), std::end(dp->local_ip_address), std::begin(value.local_ip_address));
    std::copy(std::begin(dp->remote_ip_address), std::end(dp->remote_ip_address), std::begin(value.remote_ip_address));
    value.local_port = dp->local_port;
    value.remote_port = dp->remote_port;
    value.protocol = dp->protocol;
    value.static_ip_address = dp->static_ip_address;
    std::copy(std::begin(dp->gateway_ip_address), std::end(dp->gateway_ip_address), std::begin(value.gateway_ip_address));
    std::copy(std::begin(dp->subnet_mask), std::end(dp->subnet_mask), std::begin(value.subnet_mask));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Ipv4 &ipv4)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Ipv4::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Ipv4::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, ipv4.local_ip_address);
    bytes += serialize(output, ipv4.remote_ip_address);
    bytes += serialize(output, ipv4.local_port);
    bytes += serialize(output, ipv4.remote_port);
    bytes += serialize(output, ipv4.protocol);
    bytes += serialize(output, ipv4.static_ip_address);
    bytes += serialize(output, ipv4.gateway_ip_address);
    bytes += serialize(output, ipv4.subnet_mask);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Ipv6> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_ipv6 *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Ipv6::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Ipv6::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Ipv6 value{};
    std::copy(std::begin(dp->local_ip_address), std::end(dp->local_ip_address), std::begin(value.local_ip_address));
    std::copy(std::begin(dp->remote_ip_address), std::end(dp->remote_ip_address), std::begin(value.remote_ip_address));
    value.local_port = dp->local_port;
    value.remote_port = dp->remote_port;
    value.protocol = dp->protocol;
    value.ip_address_origin = static_cast<File_path::MSG::Ipv6::IP_ADDRESS_ORIGIN>(dp->ip_address_origin);
    value.prefix_length = dp->prefix_length;
    std::copy(std::begin(dp->gateway_ip_address), std::end(dp->gateway_ip_address), std::begin(value.gateway_ip_address));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Ipv6 &ipv6)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Ipv6::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Ipv6::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, ipv6.local_ip_address);
    bytes += serialize(output, ipv6.remote_ip_address);
    bytes += serialize(output, ipv6.local_port);
    bytes += serialize(output, ipv6.remote_port);
    bytes += serialize(output, ipv6.protocol);
    bytes += serialize(output, ipv6.ip_address_origin);
    bytes += serialize(output, ipv6.prefix_length);
    bytes += serialize(output, ipv6.gateway_ip_address);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Uart> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_uart *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Uart::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Uart::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Uart value{};
    value.reserved = dp->reserved;
    value.baud_rate = dp->baud_rate;
    value.data_bits = dp->data_bits;
    value.parity = static_cast<File_path::MSG::Uart::PARITY>(dp->parity);
    value.stop_bits = static_cast<File_path::MSG::Uart::STOP_BITS>(dp->stop_bits);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Uart &uart)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Uart::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Uart::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, uart.reserved);
    bytes += serialize(output, uart.baud_rate);
    bytes += serialize(output, uart.data_bits);
    bytes += serialize(output, uart.parity);
    bytes += serialize(output, uart.stop_bits);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Usb_class> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_usb_class *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Usb_class::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Usb_class::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Usb_class value{};
    value.vendor_id = dp->vendor_id;
    value.product_id = dp->product_id;
    value.device_class = dp->device_class;
    value.device_subclass = dp->device_subclass;
    value.device_protocol = dp->device_protocol;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Usb_class &usb_class)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Usb_class::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Usb_class::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, usb_class.vendor_id);
    bytes += serialize(output, usb_class.product_id);
    bytes += serialize(output, usb_class.device_class);
    bytes += serialize(output, usb_class.device_subclass);
    bytes += serialize(output, usb_class.device_protocol);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Usb_wwid> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_usb_wwid *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Usb_wwid::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Usb_wwid::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Usb_wwid value{};
    value.interface_number = dp->interface_number;
    value.device_vendor_id = dp->device_vendor_id;
    value.device_product_id = dp->device_product_id;
    value.serial_number = reinterpret_cast<const decltype(value.serial_number)::value_type *>(dp->serial_number);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Usb_wwid &usb_wwid)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Usb_wwid::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Usb_wwid::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, usb_wwid.interface_number);
    bytes += serialize(output, usb_wwid.device_vendor_id);
    bytes += serialize(output, usb_wwid.device_product_id);
    bytes += serialize(output, usb_wwid.serial_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Device_logical_unit> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_device_logical_unit *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Device_logical_unit::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Device_logical_unit::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Device_logical_unit value{};
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Device_logical_unit &device_logical_unit)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Device_logical_unit::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Device_logical_unit::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, device_logical_unit.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Sata> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_sata *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Sata::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Sata::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Sata value{};
    value.hba_port_number = dp->hba_port_number;
    value.port_multiplier_port_number = dp->port_multiplier_port_number;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Sata &sata)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Sata::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Sata::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, sata.hba_port_number);
    bytes += serialize(output, sata.port_multiplier_port_number);
    bytes += serialize(output, sata.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Iscsi> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_iscsi *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Iscsi::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Iscsi::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Iscsi value{};
    value.protocol = dp->protocol;
    value.options = dp->options;
    value.lun = dp->lun;
    value.target_portal_group = dp->target_portal_group;
    value.target_name = reinterpret_cast<const decltype(value.target_name)::value_type *>(dp->target_name);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Iscsi &iscsi)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Iscsi::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Iscsi::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, iscsi.protocol);
    bytes += serialize(output, iscsi.options);
    bytes += serialize(output, iscsi.lun);
    bytes += serialize(output, iscsi.target_portal_group);
    bytes += serialize(output, iscsi.target_name);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Vlan> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_vlan *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Vlan::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Vlan::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Vlan value{};
    value.vlan_id = dp->vlan_id;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Vlan &vlan)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Vlan::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Vlan::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, vlan.vlan_id);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Fibre_channel_ex> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_fibre_channel_ex *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Fibre_channel_ex::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Fibre_channel_ex::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Fibre_channel_ex value{};
    value.reserved = dp->reserved;
    value.world_wide_name = dp->world_wide_name;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Fibre_channel_ex &fibre_channel_ex)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Fibre_channel_ex::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Fibre_channel_ex::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, fibre_channel_ex.reserved);
    bytes += serialize(output, fibre_channel_ex.world_wide_name);
    bytes += serialize(output, fibre_channel_ex.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Sas_extended_messaging> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_sas_extended_messaging *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Sas_extended_messaging::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Sas_extended_messaging::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Sas_extended_messaging value{};
    value.sas_address = dp->sas_address;
    value.lun = dp->lun;
    value.device_and_topology_info = dp->device_and_topology_info;
    value.relative_target_port = dp->relative_target_port;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Sas_extended_messaging &sas_extended_messaging)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Sas_extended_messaging::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Sas_extended_messaging::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, sas_extended_messaging.sas_address);
    bytes += serialize(output, sas_extended_messaging.lun);
    bytes += serialize(output, sas_extended_messaging.device_and_topology_info);
    bytes += serialize(output, sas_extended_messaging.relative_target_port);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Nvm_express_ns> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_nvm_express_ns *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Nvm_express_ns::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Nvm_express_ns::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Nvm_express_ns value{};
    value.namespace_identifier = dp->namespace_identifier;
    value.ieee_extended_unique_identifier = dp->ieee_extended_unique_identifier;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Nvm_express_ns &nvm_express_ns)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Nvm_express_ns::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Nvm_express_ns::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, nvm_express_ns.namespace_identifier);
    bytes += serialize(output, nvm_express_ns.ieee_extended_unique_identifier);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Uri> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_uri *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Uri::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Uri::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Uri value{};
    {
        static_assert(sizeof(*value.uri.data()) == sizeof(dp->uri[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->uri[0])>(advance_bytes(data, data_size)) - &dp->uri[0]);
        value.uri.resize(length);
        memcpy(value.uri.data(), dp->uri, length * sizeof(dp->uri[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Uri &uri)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Uri::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Uri::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, uri.uri);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Ufs> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_ufs *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Ufs::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Ufs::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Ufs value{};
    value.pun = dp->pun;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Ufs &ufs)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Ufs::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Ufs::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, ufs.pun);
    bytes += serialize(output, ufs.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Sd> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_sd *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Sd::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Sd::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Sd value{};
    value.slot_number = dp->slot_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Sd &sd)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Sd::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Sd::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, sd.slot_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Bluetooth> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_bluetooth *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Bluetooth::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Bluetooth::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Bluetooth value{};
    std::copy(std::begin(dp->device_address), std::end(dp->device_address), std::begin(value.device_address));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Bluetooth &bluetooth)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Bluetooth::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Bluetooth::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, bluetooth.device_address);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Wi_fi> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_wi_fi *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Wi_fi::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Wi_fi::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Wi_fi value{};
    value.ssid = reinterpret_cast<const decltype(value.ssid)::value_type *>(dp->ssid);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Wi_fi &wi_fi)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Wi_fi::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Wi_fi::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, wi_fi.ssid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Emmc> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_emmc *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Emmc::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Emmc::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Emmc value{};
    value.slot_number = dp->slot_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Emmc &emmc)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Emmc::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Emmc::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, emmc.slot_number);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Bluetoothle> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_bluetoothle *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Bluetoothle::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Bluetoothle::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Bluetoothle value{};
    std::copy(std::begin(dp->device_address), std::end(dp->device_address), std::begin(value.device_address));
    value.address_type = static_cast<File_path::MSG::Bluetoothle::ADDRESS_TYPE>(dp->address_type);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Bluetoothle &bluetoothle)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Bluetoothle::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Bluetoothle::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, bluetoothle.device_address);
    bytes += serialize(output, bluetoothle.address_type);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Dns> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_dns *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Dns::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Dns::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Dns value{};
    value.ipv6 = dp->ipv6;
    {
        static_assert(sizeof(*value.data.data()) == sizeof(dp->data[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->data[0])>(advance_bytes(data, data_size)) - &dp->data[0]);
        value.data.resize(length);
        memcpy(value.data.data(), dp->data, length * sizeof(dp->data[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Dns &dns)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Dns::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Dns::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, dns.ipv6);
    bytes += serialize(output, dns.data);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Nvdimm_ns> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_nvdimm_ns *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Nvdimm_ns::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Nvdimm_ns::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Nvdimm_ns value{};
    std::copy(std::begin(dp->uuid), std::end(dp->uuid), std::begin(value.uuid));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Nvdimm_ns &nvdimm_ns)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Nvdimm_ns::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Nvdimm_ns::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, nvdimm_ns.uuid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Rest_service> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_rest_service *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Rest_service::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Rest_service::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Rest_service value{};
    value.rest_service = static_cast<File_path::MSG::Rest_service::REST_SERVICE>(dp->rest_service);
    value.access_mode = static_cast<File_path::MSG::Rest_service::ACCESS_MODE>(dp->access_mode);
    if(static_cast<File_path::MSG::Rest_service::REST_SERVICE>(dp->rest_service) == File_path::MSG::Rest_service::REST_SERVICE::VENDOR && data_size > 6)
    {
        std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
        static_assert(sizeof(*value.data.data()) == sizeof(dp->data[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->data[0])>(advance_bytes(data, data_size)) - &dp->data[0]);
        value.data.resize(length);
        memcpy(value.data.data(), dp->data, length * sizeof(dp->data[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Rest_service &rest_service)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Rest_service::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Rest_service::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, rest_service.rest_service);
    bytes += serialize(output, rest_service.access_mode);
    if(rest_service.rest_service == File_path::MSG::Rest_service::REST_SERVICE::VENDOR)
    {
        bytes += serialize(output, rest_service.guid);
        bytes += serialize(output, rest_service.data);
    }
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MSG::Nvme_of_ns> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_msg_nvme_of_ns *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MSG::Nvme_of_ns::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MSG::Nvme_of_ns::SUBTYPE)
        return std::nullopt;

    File_path::MSG::Nvme_of_ns value{};
    value.nidt = dp->nidt;
    std::copy(std::begin(dp->nid), std::end(dp->nid), std::begin(value.nid));
    value.subsystem_nqn = reinterpret_cast<const decltype(value.subsystem_nqn)::value_type *>(dp->subsystem_nqn);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MSG::Nvme_of_ns &nvme_of_ns)
{
    size_t bytes = 0;
    uint8_t type = File_path::MSG::Nvme_of_ns::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MSG::Nvme_of_ns::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, nvme_of_ns.nidt);
    bytes += serialize(output, nvme_of_ns.nid);
    bytes += serialize(output, nvme_of_ns.subsystem_nqn);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

// Media

template <>
inline std::optional<File_path::MEDIA::Hd> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_hd *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Hd::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Hd::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Hd value{};
    value.partition_number = dp->partition_number;
    value.partition_start = dp->partition_start;
    value.partition_size = dp->partition_size;
    std::copy(std::begin(dp->partition_signature), std::end(dp->partition_signature), std::begin(value.partition_signature));
    value.partition_format = static_cast<File_path::MEDIA::Hd::PARTITION_FORMAT>(dp->partition_format);
    value.signature_type = static_cast<File_path::MEDIA::Hd::SIGNATURE_TYPE>(dp->signature_type);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Hd &hd)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Hd::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Hd::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, hd.partition_number);
    bytes += serialize(output, hd.partition_start);
    bytes += serialize(output, hd.partition_size);
    bytes += serialize(output, hd.partition_signature);
    bytes += serialize(output, hd.partition_format);
    bytes += serialize(output, hd.signature_type);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Cd_rom> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_cd_rom *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Cd_rom::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Cd_rom::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Cd_rom value{};
    value.boot_entry = dp->boot_entry;
    value.partition_start = dp->partition_start;
    value.partition_size = dp->partition_size;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Cd_rom &cd_rom)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Cd_rom::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Cd_rom::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, cd_rom.boot_entry);
    bytes += serialize(output, cd_rom.partition_start);
    bytes += serialize(output, cd_rom.partition_size);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Vendor> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_vendor *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Vendor::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Vendor::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Vendor value{};
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    {
        static_assert(sizeof(*value.data.data()) == sizeof(dp->data[0]));
        auto length = static_cast<size_t>(static_cast<decltype(&dp->data[0])>(advance_bytes(data, data_size)) - &dp->data[0]);
        value.data.resize(length);
        memcpy(value.data.data(), dp->data, length * sizeof(dp->data[0]));
    }
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Vendor &vendor)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Vendor::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Vendor::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, vendor.guid);
    bytes += serialize(output, vendor.data);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::File_path> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_file_path *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::File_path::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::File_path::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::File_path value{};
    value.path_name = reinterpret_cast<const decltype(value.path_name)::value_type *>(dp->path_name);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::File_path &file_path)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::File_path::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::File_path::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, file_path.path_name);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Protocol> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_protocol *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Protocol::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Protocol::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Protocol value{};
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Protocol &protocol)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Protocol::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Protocol::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, protocol.guid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Firmware_file> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_firmware_file *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Firmware_file::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Firmware_file::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Firmware_file value{};
    std::copy(std::begin(dp->name), std::end(dp->name), std::begin(value.name));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Firmware_file &firmware_file)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Firmware_file::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Firmware_file::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, firmware_file.name);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Firmware_volume> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_firmware_volume *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Firmware_volume::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Firmware_volume::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Firmware_volume value{};
    std::copy(std::begin(dp->name), std::end(dp->name), std::begin(value.name));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Firmware_volume &firmware_volume)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Firmware_volume::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Firmware_volume::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, firmware_volume.name);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Relative_offset_range> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_relative_offset_range *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Relative_offset_range::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Relative_offset_range::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Relative_offset_range value{};
    value.reserved = dp->reserved;
    value.starting_offset = dp->starting_offset;
    value.ending_offset = dp->ending_offset;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Relative_offset_range &relative_offset_range)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Relative_offset_range::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Relative_offset_range::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, relative_offset_range.reserved);
    bytes += serialize(output, relative_offset_range.starting_offset);
    bytes += serialize(output, relative_offset_range.ending_offset);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::MEDIA::Ram_disk> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_media_ram_disk *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::MEDIA::Ram_disk::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::MEDIA::Ram_disk::SUBTYPE)
        return std::nullopt;

    File_path::MEDIA::Ram_disk value{};
    value.starting_address = dp->starting_address;
    value.ending_address = dp->ending_address;
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    value.disk_instance = dp->disk_instance;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::MEDIA::Ram_disk &ram_disk)
{
    size_t bytes = 0;
    uint8_t type = File_path::MEDIA::Ram_disk::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::MEDIA::Ram_disk::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, ram_disk.starting_address);
    bytes += serialize(output, ram_disk.ending_address);
    bytes += serialize(output, ram_disk.guid);
    bytes += serialize(output, ram_disk.disk_instance);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

// BIOS

template <>
inline std::optional<File_path::BIOS::Boot_specification> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_bios_boot_specification *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::BIOS::Boot_specification::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::BIOS::Boot_specification::SUBTYPE)
        return std::nullopt;

    File_path::BIOS::Boot_specification value{};
    value.device_type = dp->device_type;
    value.status_flag = dp->status_flag;
    value.description = reinterpret_cast<const decltype(value.description)::value_type *>(dp->description);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::BIOS::Boot_specification &boot_specification)
{
    size_t bytes = 0;
    uint8_t type = File_path::BIOS::Boot_specification::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::BIOS::Boot_specification::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, boot_specification.device_type);
    bytes += serialize(output, boot_specification.status_flag);
    bytes += serialize(output, boot_specification.description);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

// End

template <>
inline std::optional<File_path::END::Instance> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_end_instance *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::END::Instance::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::END::Instance::SUBTYPE)
        return std::nullopt;

    File_path::END::Instance value{};
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::END::Instance &)
{
    size_t bytes = 0;
    uint8_t type = File_path::END::Instance::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::END::Instance::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::END::Entire> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_end_entire *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != File_path::END::Entire::TYPE)
        return std::nullopt;

    if(dp->header.subtype != File_path::END::Entire::SUBTYPE)
        return std::nullopt;

    File_path::END::Entire value{};
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::END::Entire &)
{
    size_t bytes = 0;
    uint8_t type = File_path::END::Entire::TYPE;
    bytes += serialize(output, type);
    uint8_t subtype = File_path::END::Entire::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::Unknown> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const efidp_header *>(data);
    if(dp->length != data_size)
        return std::nullopt;

    File_path::Unknown value{};
    value.TYPE = dp->type;
    value.SUBTYPE = dp->subtype;

    size_t data_length = data_size - sizeof(*dp);
    value.data.resize(data_length);
    memcpy(value.data.data(), static_cast<const uint8_t *>(advance_bytes(data, sizeof(const efidp_header))), data_length);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const File_path::Unknown &unknown)
{
    size_t bytes = 0;
    bytes += serialize(output, unknown.TYPE);
    bytes += serialize(output, unknown.SUBTYPE);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, unknown.data);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<File_path::ANY> deserialize(const void *data, size_t data_size)
{
    auto dp = static_cast<const_efidp>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

#define TYPE_SUBTYPE(type, subtype) (((type) << 8) | (subtype))
#define casefp(Type)                                                    \
    case TYPE_SUBTYPE(File_path::Type::TYPE, File_path::Type::SUBTYPE): \
        return deserialize<File_path::Type>(dp, data_size)

    switch(TYPE_SUBTYPE(dp->header.type, dp->header.subtype))
    {
        casefp(HW::Pci);
        casefp(HW::Pccard);
        casefp(HW::Memory_mapped);
        casefp(HW::Vendor);
        casefp(HW::Controller);
        casefp(HW::Bmc);
        casefp(ACPI::Acpi);
        casefp(ACPI::Expanded);
        casefp(ACPI::Adr);
        casefp(ACPI::Nvdimm);
        casefp(MSG::Atapi);
        casefp(MSG::Scsi);
        casefp(MSG::Fibre_channel);
        casefp(MSG::Firewire);
        casefp(MSG::Usb);
        casefp(MSG::I2o);
        casefp(MSG::Infiniband);
        casefp(MSG::Vendor);
        casefp(MSG::Mac_address);
        casefp(MSG::Ipv4);
        casefp(MSG::Ipv6);
        casefp(MSG::Uart);
        casefp(MSG::Usb_class);
        casefp(MSG::Usb_wwid);
        casefp(MSG::Device_logical_unit);
        casefp(MSG::Sata);
        casefp(MSG::Iscsi);
        casefp(MSG::Vlan);
        casefp(MSG::Fibre_channel_ex);
        casefp(MSG::Sas_extended_messaging);
        casefp(MSG::Nvm_express_ns);
        casefp(MSG::Uri);
        casefp(MSG::Ufs);
        casefp(MSG::Sd);
        casefp(MSG::Bluetooth);
        casefp(MSG::Wi_fi);
        casefp(MSG::Emmc);
        casefp(MSG::Bluetoothle);
        casefp(MSG::Dns);
        casefp(MSG::Nvdimm_ns);
        casefp(MSG::Rest_service);
        casefp(MSG::Nvme_of_ns);
        casefp(MEDIA::Hd);
        casefp(MEDIA::Cd_rom);
        casefp(MEDIA::Vendor);
        casefp(MEDIA::File_path);
        casefp(MEDIA::Protocol);
        casefp(MEDIA::Firmware_file);
        casefp(MEDIA::Firmware_volume);
        casefp(MEDIA::Relative_offset_range);
        casefp(MEDIA::Ram_disk);
        casefp(BIOS::Boot_specification);
        casefp(END::Instance);
        casefp(END::Entire);

    default:
        return deserialize<File_path::Unknown>(dp, data_size);
    }

#undef casefp
#undef TYPE_SUBTYPE
}

template <>
inline size_t serialize(Raw_data &output, const File_path::ANY &file_path)
{
    return std::visit([&output](const auto &dp) -> size_t
        { return serialize(output, dp); },
        file_path);
}

template <>
inline std::optional<Load_option> deserialize(const void *data, size_t data_size)
{
    Load_option value{};
    auto ssize = static_cast<ssize_t>(data_size);
    auto load_option = const_cast<efi_load_option *>(static_cast<const efi_load_option *>(data));

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
    for(size_t d = 0; load_option->description[d]; ++d)
        value.description.push_back(load_option->description[d]);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    uint16_t device_path_size = efi_loadopt_pathlen(load_option, ssize);
    const_efidp device_path = efi_loadopt_path(load_option, ssize);

    auto file_paths = deserialize_list_ex<File_path::ANY>(
        device_path, device_path_size,
        [](const void *ptr)
        {
            auto size = efidp_node_size(static_cast<const_efidp>(ptr));
            return static_cast<size_t>(size);
        },
        [](const void *ptr, size_t bytes_left) -> const void *
        {
            auto dp = static_cast<const_efidp>(ptr);
            ssize_t size = efidp_node_size(dp);
            if(size < 0 || static_cast<size_t>(size) > bytes_left)
                return nullptr;

            return advance_bytes(ptr, static_cast<size_t>(size));
        });

    if(!file_paths || file_paths->empty())
        return std::nullopt;

    value.device_path = *file_paths;

    uint8_t *optional_data = nullptr;
    size_t optional_data_size = 0;
    if(int ret = efi_loadopt_optional_data(load_option, data_size, &optional_data, &optional_data_size); ret >= 0)
    {
        auto opt_data = deserialize<Raw_data>(optional_data, optional_data_size);
        if(!opt_data)
            return std::nullopt;

        value.optional_data = *opt_data;
    }

    value.attributes = static_cast<Load_option_attribute>(load_option->attributes);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Load_option &load_option)
{
    size_t size = 0;
    size += serialize(output, load_option.attributes);
    auto file_path_list_length_pos = output.size();
    uint16_t file_path_list_size = 0;
    size += serialize(output, file_path_list_size);
    size += serialize(output, load_option.description);

    file_path_list_size = static_cast<uint16_t>(serialize_list(output, load_option.device_path));
    // Always set END_ENTIRE tag at the end of device path
    if(!load_option.device_path.size() || std::visit([](const auto &file_path)
                                              { return file_path.SUBTYPE; },
                                              load_option.device_path.back())
            != File_path::END::Entire::SUBTYPE)
    {
        File_path::END::Entire end{};
        file_path_list_size = static_cast<uint16_t>(file_path_list_size + static_cast<uint16_t>(serialize(output, end))); // Older GCC complains about conversion when using += `conversion from ‘int’ to ‘uint16_t’ {aka ‘short unsigned int’} may change value`
    }
    size += file_path_list_size;
    memcpy(&output[file_path_list_length_pos], &file_path_list_size, sizeof(file_path_list_size));

    size += serialize(output, load_option.optional_data);
    return size;
}

extern Progress_fn _get_variables_progress_fn;

template <class Filter_fn>
inline std::unordered_map<tstring, efi_guid_t> get_variables(const Filter_fn &filter_fn, const Progress_fn &progress_fn)
{
    std::unordered_map<tstring, efi_guid_t> variables;
    efi_guid_t *guid = nullptr;
    TCHAR *name = nullptr;
    _get_variables_progress_fn = progress_fn;
    efi_set_get_next_variable_name_progress_cb([](size_t step, size_t total) noexcept
        { try { if(_get_variables_progress_fn)_get_variables_progress_fn(step, total); } catch (...) {/* ignore */} });

    while(efi_get_next_variable_name(&guid, &name) > 0)
    {
        if(!filter_fn(*guid, name))
            continue;

        memcpy(&variables[name], guid, sizeof(efi_guid_t));
    }

    efi_set_get_next_variable_name_progress_cb(nullptr);
    _get_variables_progress_fn = nullptr;
    return variables;
}

template <class Filter_fn>
inline std::unordered_map<tstring, efi_guid_t> get_variables(Filter_fn filter_fn)
{
    return get_variables(filter_fn, [](size_t, size_t) { /* noprogress */ });
}

inline std::unordered_map<tstring, efi_guid_t> get_variables()
{
    return get_variables(
        [](const efi_guid_t &, const tstring_view)
        { return true; },
        [](size_t, size_t) { /* noprogress */ });
}

template <class Type>
inline std::optional<Variable<Type>> get_variable(const efi_guid_t &guid, const tstring &name)
{
    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    if(int ret = efi_get_variable(guid, name.c_str(), &data, &data_size, &attributes); ret < 0)
        return std::nullopt;

    auto value = deserialize<Type>(data, data_size);
    if(!value)
        return std::nullopt;

    return {{*value, attributes}};
}

template <class Type>
inline std::optional<Variable<std::vector<Type>>> get_list_variable(const efi_guid_t &guid, const tstring &name)
{
    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    if(int ret = efi_get_variable(guid, name.c_str(), &data, &data_size, &attributes); ret < 0)
        return std::nullopt;

    auto value = deserialize_list<Type>(data, data_size);
    if(!value)
        return std::nullopt;

    return {{*value, attributes}};
}

template <class Type, class Size_fn, class Advance_fn>
inline std::optional<Variable<std::vector<Type>>> get_list_variable_ex(const efi_guid_t &guid, const tstring &name, const Size_fn &get_element_size, const Advance_fn &get_next_element)
{
    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    if(int ret = efi_get_variable(guid, name.c_str(), &data, &data_size, &attributes); ret < 0)
        return std::nullopt;

    auto value = deserialize_list_ex<Type>(data, data_size, get_element_size, get_next_element);
    if(!value)
        return std::nullopt;

    return {{*value, attributes}};
}

template <class Type>
inline bool set_variable(const efi_guid_t &guid, const tstring &name, const Variable<Type> &variable, mode_t mode)
{
    auto [value, attributes] = variable;
    Raw_data bytes;
    size_t size = serialize(bytes, value);
    // Skip overwriting with exactly the same value
    if(const auto current = EFIBoot::get_variable<Raw_data>(guid, name); current)
    {
        const auto &[current_bytes, current_attributes] = *current;
        if(current_attributes == attributes && current_bytes == bytes)
            return true;
    }

    // Don't care about the error from get_variable
    efi_error_clear();
    return efi_set_variable(guid, name.c_str(), bytes.data(), size, attributes, mode) == 0;
}

template <class Type>
inline bool set_list_variable(const efi_guid_t &guid, const tstring &name, const Variable<std::vector<Type>> &variable, mode_t mode)
{
    auto [value, attributes] = variable;
    Raw_data bytes;
    size_t size = serialize_list(bytes, value);
    // Skip overwriting with exactly the same value
    if(const auto current = EFIBoot::get_variable<Raw_data>(guid, name); current)
    {
        const auto &[current_bytes, current_attributes] = *current;
        if(current_attributes == attributes && current_bytes == bytes)
            return true;
    }

    // Don't care about the error from get_variable
    efi_error_clear();
    return efi_set_variable(guid, name.c_str(), bytes.data(), size, attributes, mode) == 0;
}

inline bool del_variable(const efi_guid_t &guid, const tstring &name)
{
    return efi_del_variable(guid, name.c_str()) == 0;
}

inline tstring get_error_trace()
{
    tstring output = _T("Error trace:\n");
    int rc = 1;
    for(unsigned int i = 0; rc > 0; i++)
    {
        TCHAR *filename = nullptr;
        TCHAR *function = nullptr;
        int line = 0;
        TCHAR *message = nullptr;
        int error = 0;
        const int ERROR_STR_BUFFER_SIZE = 1024;
        TCHAR error_str[ERROR_STR_BUFFER_SIZE] = {};

        rc = efi_error_get(i, &filename, &function, &line, &message, &error);
        if(rc < 0)
            output += _T("error fetching trace value\n");

        if(rc == 0)
            break;

        rc = _tcserror_s(error_str, ERROR_STR_BUFFER_SIZE - 1, error);
        if(rc != 0)
            output += _T("error translating error code to string\n");

        output += filename;
        output += _T(":");
        output += to_tstring(line);
        output += _T(" ");
        output += function;
        output += _T("(): ");
        output += error_str;
        output += _T(": ");
        output += message;
        output += _T("\n");
    }

    return output;
}

} // namespace EFIBoot
