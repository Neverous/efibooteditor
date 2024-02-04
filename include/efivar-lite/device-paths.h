// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <errno.h>
#include <limits.h>

#include "efivar-lite.h"

#pragma pack(push, 1)
typedef struct
{
    uint8_t type;
    uint8_t subtype;
    uint16_t length;
} efidp_header;

enum EFIDP_TYPE
{
    EFIDP_TYPE_HW = 0x01,
    EFIDP_TYPE_ACPI = 0x02,
    EFIDP_TYPE_MSG = 0x03,
    EFIDP_TYPE_MEDIA = 0x04,
    EFIDP_TYPE_BIOS = 0x05,
    EFIDP_TYPE_END = 0x7f,
};

enum EFIDP_HW
{
    EFIDP_HW_PCI = 0x01,
    EFIDP_HW_PCCARD = 0x02,
    EFIDP_HW_MEMORY_MAPPED = 0x03,
    EFIDP_HW_VENDOR = 0x04,
    EFIDP_HW_CONTROLLER = 0x05,
    EFIDP_HW_BMC = 0x06,
};

enum EFIDP_ACPI
{
    EFIDP_ACPI_ACPI = 0x01,
    EFIDP_ACPI_EXPANDED = 0x02,
    EFIDP_ACPI_ADR = 0x03,
    EFIDP_ACPI_NVDIMM = 0x04,
};

enum EFIDP_MSG
{
    EFIDP_MSG_ATAPI = 0x01,
    EFIDP_MSG_SCSI = 0x02,
    EFIDP_MSG_FIBRE_CHANNEL = 0x03,
    EFIDP_MSG_FIREWIRE = 0x04,
    EFIDP_MSG_USB = 0x05,
    EFIDP_MSG_I2O = 0x06,
    EFIDP_MSG_INFINIBAND = 0x09,
    EFIDP_MSG_VENDOR = 0x0a,
    EFIDP_MSG_MAC_ADDRESS = 0x0b,
    EFIDP_MSG_IPV4 = 0x0c,
    EFIDP_MSG_IPV6 = 0x0d,
    EFIDP_MSG_UART = 0x0e,
    EFIDP_MSG_USB_CLASS = 0x0f,
    EFIDP_MSG_USB_WWID = 0x10,
    EFIDP_MSG_DEVICE_LOGICAL_UNIT = 0x11,
    EFIDP_MSG_SATA = 0x12,
    EFIDP_MSG_ISCSI = 0x13,
    EFIDP_MSG_VLAN = 0x14,
    EFIDP_MSG_FIBRE_CHANNEL_EX = 0x15,
    EFIDP_MSG_SAS_EXTENDED_MESSAGING = 0x16,
    EFIDP_MSG_NVM_EXPRESS_NS = 0x17,
    EFIDP_MSG_URI = 0x18,
    EFIDP_MSG_UFS = 0x19,
    EFIDP_MSG_SD = 0x1a,
    EFIDP_MSG_BLUETOOTH = 0x1b,
    EFIDP_MSG_WI_FI = 0x1c,
    EFIDP_MSG_EMMC = 0x1d,
    EFIDP_MSG_BLUETOOTHLE = 0x1e,
    EFIDP_MSG_DNS = 0x1f,
    EFIDP_MSG_NVDIMM_NS = 0x20,
    EFIDP_MSG_REST_SERVICE = 0x21,
    EFIDP_MSG_NVME_OF_NS = 0x22,
};

enum EFIDP_MEDIA
{
    EFIDP_MEDIA_HD = 0x01,
    EFIDP_MEDIA_CD_ROM = 0x02,
    EFIDP_MEDIA_VENDOR = 0x03,
    EFIDP_MEDIA_FILE_PATH = 0x04,
    EFIDP_MEDIA_PROTOCOL = 0x05,
    EFIDP_MEDIA_FIRMWARE_FILE = 0x06,
    EFIDP_MEDIA_FIRMWARE_VOLUME = 0x07,
    EFIDP_MEDIA_RELATIVE_OFFSET_RANGE = 0x08,
    EFIDP_MEDIA_RAM_DISK = 0x09,
};

enum EFIDP_BIOS
{
    EFIDP_BIOS_BOOT_SPECIFICATION = 0x01,
};

enum EFIDP_END
{
    EFIDP_END_INSTANCE = 0x01,
    EFIDP_END_ENTIRE = 0xff,
};

/*
   Hardware
   This Device Path defines how a device is attached to the resource domain of a system, where resource domain is simply the shared memory, memory mapped I/O, and I/O space of the system.
*/
/*
   PCI
   The Device Path for PCI defines the path to the PCI configuration space address for a PCI device.
*/
typedef struct
{
    efidp_header header;
    uint8_t function;
    uint8_t device;
} efidp_hw_pci;

/*
   PCCARD
   PCCARD Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t function_number;
} efidp_hw_pccard;

enum EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE
{
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_RESERVED = 0x0,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_LOADER_CODE = 0x1,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_LOADER_DATA = 0x2,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_BOOT_SERVICES_CODE = 0x3,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_BOOT_SERVICES_DATA = 0x4,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_RUNTIME_SERVICES_CODE = 0x5,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_RUNTIME_SERVICES_DATA = 0x6,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_CONVENTIONAL = 0x7,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_UNUSABLE = 0x8,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_ACPI_RECLAIM = 0x9,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_ACPI_MEMORY_NVS = 0xa,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_MEMORY_MAPPED_IO = 0xb,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_MEMORY_MAPPD_IO_PORT_SPACE = 0xc,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_PAL_CODE = 0xd,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_PERSISTENT = 0xe,
    EFIDP_HW_MEMORY_MAPPED_MEMORY_TYPE_UNACCEPTED = 0xf,
};

/*
   Memory Mapped
   Memory Mapped Settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t memory_type;
    uint64_t start_address;
    uint64_t end_address;
} efidp_hw_memory_mapped;

/*
   Vendor-Defined Hardware
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
typedef struct
{
    efidp_header header;
    uint8_t guid[16];
    uint8_t data[ANYSIZE_ARRAY];
} efidp_hw_vendor;

/*
   Controller
   Controller settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t controller_number;
} efidp_hw_controller;

enum EFIDP_HW_BMC_INTERFACE_TYPE
{
    EFIDP_HW_BMC_INTERFACE_TYPE_UNKNOWN = 0x0,
    EFIDP_HW_BMC_INTERFACE_TYPE_KCS = 0x1,
    EFIDP_HW_BMC_INTERFACE_TYPE_SMIC = 0x2,
    EFIDP_HW_BMC_INTERFACE_TYPE_BT = 0x3,
};

/*
   BMC
   The Device Path for a Baseboard Management Controller (BMC) host interface.
*/
typedef struct
{
    efidp_header header;
    uint8_t interface_type;
    uint64_t base_address;
} efidp_hw_bmc;

/*
   ACPI
   This Device Path is used to describe devices whose enumeration is not described in an industry-standard fashion. These devices must be described using ACPI AML in the ACPI name space; this Device Path is a linkage to the ACPI name space.
*/
/*
   ACPI
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
typedef struct
{
    efidp_header header;
    uint32_t hid;
    uint32_t uid;
} efidp_acpi_acpi;

/*
   Expanded
   This Device Path contains ACPI Device IDs that represent a device’s Plug and Play Hardware ID and its corresponding unique persistent ID.
*/
typedef struct
{
    efidp_header header;
    uint32_t hid;
    uint32_t uid;
    uint32_t cid;
    uint8_t hidstr[ANYSIZE_ARRAY];
    // uint8_t uidstr[ANYSIZE_ARRAY];
    // uint8_t cidstr[ANYSIZE_ARRAY];
} efidp_acpi_expanded;

/*
   ADR
   The ADR device path is used to contain video output device attributes to support the Graphics Output Protocol.
*/
typedef struct
{
    efidp_header header;
    uint32_t adr;
    uint8_t additional_adr[ANYSIZE_ARRAY];
} efidp_acpi_adr;

/*
   NVDIMM
   This device path describes an NVDIMM device using the ACPI 6.0 specification defined NFIT Device Handle as the identifier.
*/
typedef struct
{
    efidp_header header;
    uint32_t nfit_device_handle;
} efidp_acpi_nvdimm;

/*
   Messaging
   This Device Path is used to describe the connection of devices outside the resource domain of the system. This Device Path can describe physical messaging information such as a SCSI ID, or abstract information such as networking protocol IP addresses.
*/
/*
   ATAPI
   ATAPI Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t primary;
    uint8_t slave;
    uint16_t lun;
} efidp_msg_atapi;

/*
   SCSI
   SCSI Settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t pun;
    uint16_t lun;
} efidp_msg_scsi;

/*
   Fibre Channel
   Fibre Channel Settings
*/
typedef struct
{
    efidp_header header;
    uint32_t reserved;
    uint64_t world_wide_name;
    uint64_t lun;
} efidp_msg_fibre_channel;

/*
   Firewire
   Firewire Settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t reserved;
    uint64_t guid;
} efidp_msg_firewire;

/*
   USB
   USB settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t parent_port_number;
    uint8_t interface_number;
} efidp_msg_usb;

/*
   I2O
   I2O Settings
*/
typedef struct
{
    efidp_header header;
    uint32_t tid;
} efidp_msg_i2o;

/*
   InfiniBand
   InfiniBand Settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t resource_flags;
    uint8_t port_gid[16];
    uint64_t ioc_guid_service_id;
    uint64_t target_port_id;
    uint64_t device_id;
} efidp_msg_infiniband;

/*
   Vendor-Defined Messaging
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
typedef struct
{
    efidp_header header;
    uint8_t guid[16];
    uint8_t data[ANYSIZE_ARRAY];
} efidp_msg_vendor;

/*
   MAC Address
   MAC settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t address[32];
    uint8_t if_type;
} efidp_msg_mac_address;

/*
   IPv4
   IPv4 settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t local_ip_address[4];
    uint8_t remote_ip_address[4];
    uint16_t local_port;
    uint16_t remote_port;
    uint16_t protocol;
    uint8_t static_ip_address;
    uint8_t gateway_ip_address[4];
    uint8_t subnet_mask[4];
} efidp_msg_ipv4;

enum EFIDP_MSG_IPV6_IP_ADDRESS_ORIGIN
{
    EFIDP_MSG_IPV6_IP_ADDRESS_ORIGIN_STATIC = 0x0,
    EFIDP_MSG_IPV6_IP_ADDRESS_ORIGIN_STATELESS = 0x1,
    EFIDP_MSG_IPV6_IP_ADDRESS_ORIGIN_STATEFUL = 0x2,
};

/*
   IPv6
   IPv6 settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t local_ip_address[16];
    uint8_t remote_ip_address[16];
    uint16_t local_port;
    uint16_t remote_port;
    uint16_t protocol;
    uint8_t ip_address_origin;
    uint8_t prefix_length;
    uint8_t gateway_ip_address[16];
} efidp_msg_ipv6;

enum EFIDP_MSG_UART_PARITY
{
    EFIDP_MSG_UART_PARITY_DEFAULT = 0x0,
    EFIDP_MSG_UART_PARITY_NO = 0x1,
    EFIDP_MSG_UART_PARITY_EVEN = 0x2,
    EFIDP_MSG_UART_PARITY_ODD = 0x3,
    EFIDP_MSG_UART_PARITY_MARK = 0x4,
    EFIDP_MSG_UART_PARITY_SPACE = 0x5,
};

enum EFIDP_MSG_UART_STOP_BITS
{
    EFIDP_MSG_UART_STOP_BITS_DEFAULT = 0x0,
    EFIDP_MSG_UART_STOP_BITS_ONE = 0x1,
    EFIDP_MSG_UART_STOP_BITS_ONE_AND_HALF = 0x2,
    EFIDP_MSG_UART_STOP_BITS_TWO = 0x3,
};

/*
   UART
   UART Settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t reserved;
    uint64_t baud_rate;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
} efidp_msg_uart;

/*
   USB Class
   USB Class Settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t device_protocol;
} efidp_msg_usb_class;

/*
   USB WWID
   This device path describes a USB device using its serial number.
*/
typedef struct
{
    efidp_header header;
    uint16_t interface_number;
    uint16_t device_vendor_id;
    uint16_t device_product_id;
    uint16_t serial_number[ANYSIZE_ARRAY];
} efidp_msg_usb_wwid;

/*
   Device Logical Unit
   Device Logical Unit Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t lun;
} efidp_msg_device_logical_unit;

/*
   SATA
   SATA settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t hba_port_number;
    uint16_t port_multiplier_port_number;
    uint16_t lun;
} efidp_msg_sata;

/*
   iSCSI
   iSCSI Settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t protocol;
    uint16_t options;
    uint64_t lun;
    uint16_t target_portal_group;
    uint8_t target_name[ANYSIZE_ARRAY];
} efidp_msg_iscsi;

/*
   VLAN
   VLAN Settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t vlan_id;
} efidp_msg_vlan;

/*
   Fibre Channel Ex
   The Fibre Channel Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
typedef struct
{
    efidp_header header;
    uint32_t reserved;
    uint64_t world_wide_name;
    uint64_t lun;
} efidp_msg_fibre_channel_ex;

/*
   SAS Extended Messaging
   The SAS Ex device path clarifies the definition of the Logical Unit Number field to conform with the T-10 SCSI Architecture Model 4 specification.
*/
typedef struct
{
    efidp_header header;
    uint64_t sas_address;
    uint64_t lun;
    uint16_t device_and_topology_info;
    uint16_t relative_target_port;
} efidp_msg_sas_extended_messaging;

/*
   NVM Express NS
   NVM Express Namespace Settings.
*/
typedef struct
{
    efidp_header header;
    uint32_t namespace_identifier;
    uint64_t ieee_extended_unique_identifier;
} efidp_msg_nvm_express_ns;

/*
   URI
   Refer to RFC 3986 for details on the URI contents.
*/
typedef struct
{
    efidp_header header;
    uint8_t uri[ANYSIZE_ARRAY];
} efidp_msg_uri;

/*
   UFS
   UFS Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t pun;
    uint8_t lun;
} efidp_msg_ufs;

/*
   SD
   SD Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t slot_number;
} efidp_msg_sd;

/*
   Bluetooth
   EFI Bluetooth Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t device_address[6];
} efidp_msg_bluetooth;

/*
   Wi-Fi
   Wi-Fi Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t ssid;
} efidp_msg_wi_fi;

/*
   eMMC
   Embedded Multi-Media Card Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t slot_number;
} efidp_msg_emmc;

enum EFIDP_MSG_BLUETOOTHLE_ADDRESS_TYPE
{
    EFIDP_MSG_BLUETOOTHLE_ADDRESS_TYPE_PUBLIC = 0x0,
    EFIDP_MSG_BLUETOOTHLE_ADDRESS_TYPE_RANDOM = 0x1,
};

/*
   BluetoothLE
   EFI BluetoothLE Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t device_address[6];
    uint8_t address_type;
} efidp_msg_bluetoothle;

/*
   DNS
   DNS Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t ipv6;
    uint8_t data[ANYSIZE_ARRAY];
} efidp_msg_dns;

/*
   NVDIMM NS
   This device path describes a bootable NVDIMM namespace that is defined by a namespace label.
*/
typedef struct
{
    efidp_header header;
    uint8_t uuid[16];
} efidp_msg_nvdimm_ns;

enum EFIDP_MSG_REST_SERVICE_REST_SERVICE
{
    EFIDP_MSG_REST_SERVICE_REST_SERVICE_REDFISH = 0x1,
    EFIDP_MSG_REST_SERVICE_REST_SERVICE_ODATA = 0x2,
    EFIDP_MSG_REST_SERVICE_REST_SERVICE_VENDOR = 0xff,
};

enum EFIDP_MSG_REST_SERVICE_ACCESS_MODE
{
    EFIDP_MSG_REST_SERVICE_ACCESS_MODE_IN_BAND = 0x1,
    EFIDP_MSG_REST_SERVICE_ACCESS_MODE_OUT_OF_BAND = 0x2,
};

/*
   REST Service
   REST Service Settings.
*/
typedef struct
{
    efidp_header header;
    uint8_t rest_service;
    uint8_t access_mode;
    uint8_t guid[16];
    uint8_t data[ANYSIZE_ARRAY];
} efidp_msg_rest_service;

/*
   NVMe-oF NS
   This device path describes a bootable NVMe over Fiber namespace that is defined by a unique Namespace and Subsystem NQN identity.
*/
typedef struct
{
    efidp_header header;
    uint8_t nidt;
    uint8_t nid[16];
    uint8_t subsystem_nqn[ANYSIZE_ARRAY];
} efidp_msg_nvme_of_ns;

/*
   Media
   This Device Path is used to describe the portion of a medium that is being abstracted by a boot service. For example, a Media Device Path could define which partition on a hard drive was being used.
*/
enum EFIDP_MEDIA_HD_PARTITION_FORMAT
{
    EFIDP_MEDIA_HD_PARTITION_FORMAT_MBR = 0x1,
    EFIDP_MEDIA_HD_PARTITION_FORMAT_GUID = 0x2,
};

enum EFIDP_MEDIA_HD_SIGNATURE_TYPE
{
    EFIDP_MEDIA_HD_SIGNATURE_TYPE_NONE = 0x0,
    EFIDP_MEDIA_HD_SIGNATURE_TYPE_MBR = 0x1,
    EFIDP_MEDIA_HD_SIGNATURE_TYPE_GUID = 0x2,
};

/*
   Hard Drive
   The Hard Drive Media Device Path is used to represent a partition on a hard drive.
*/
typedef struct
{
    efidp_header header;
    uint32_t partition_number;
    uint64_t partition_start;
    uint64_t partition_size;
    uint8_t partition_signature[16];
    uint8_t partition_format;
    uint8_t signature_type;
} efidp_media_hd;

/*
   CD-ROM
   The CD-ROM Media Device Path is used to define a system partition that exists on a CD-ROM.
*/
typedef struct
{
    efidp_header header;
    uint32_t boot_entry;
    uint64_t partition_start;
    uint64_t partition_size;
} efidp_media_cd_rom;

/*
   Vendor-Defined Media
   The Vendor Device Path allows the creation of vendor-defined Device Paths.
*/
typedef struct
{
    efidp_header header;
    uint8_t guid[16];
    uint8_t data[ANYSIZE_ARRAY];
} efidp_media_vendor;

/*
   File Path
   File Path settings.
*/
typedef struct
{
    efidp_header header;
    uint16_t path_name[ANYSIZE_ARRAY];
} efidp_media_file_path;

/*
   Protocol
   The Media Protocol Device Path is used to denote the protocol that is being used in a device path at the location of the path specified.
*/
typedef struct
{
    efidp_header header;
    uint8_t guid[16];
} efidp_media_protocol;

/*
   Firmware File
   Describes a firmware file in a firmware volume.
*/
typedef struct
{
    efidp_header header;
    uint8_t name[16];
} efidp_media_firmware_file;

/*
   Firmware Volume
   Describes a firmware volume.
*/
typedef struct
{
    efidp_header header;
    uint8_t name[16];
} efidp_media_firmware_volume;

/*
   Relative Offset Range
   This device path node specifies a range of offsets relative to the first byte available on the device.
*/
typedef struct
{
    efidp_header header;
    uint32_t reserved;
    uint64_t starting_offset;
    uint64_t ending_offset;
} efidp_media_relative_offset_range;

/*
   RAM Disk
   RAM Disk Settings.
*/
typedef struct
{
    efidp_header header;
    uint64_t starting_address;
    uint64_t ending_address;
    uint8_t guid[16];
    uint16_t disk_instance;
} efidp_media_ram_disk;

/*
   BIOS
   This Device Path is used to point to boot legacy operating systems. it is based on the BIOS Boot Specification Version 1.01.
*/
/*
   BIOS Boot Specification
   This Device Path is used to describe the booting of non-EFI-aware operating systems.
*/
typedef struct
{
    efidp_header header;
    uint16_t device_type;
    uint16_t status_flag;
    uint8_t description[ANYSIZE_ARRAY];
} efidp_bios_boot_specification;

/*
   End
   Depending on the Sub-Type, this Device Path node is used to indicate the end of the Device Path instance or Device Path structure.
*/
/*
   End This Instance
   This type of node terminates one Device Path instance and denotes the start of another. This is only required when an environment variable represents multiple devices.
*/
typedef struct
{
    efidp_header header;
} efidp_end_instance;

/*
   End Entire
   This type of node terminates an entire Device Path. Software searches for this sub-type to find the end of a Device Path. All Device Paths must end with this sub-type.
*/
typedef struct
{
    efidp_header header;
} efidp_end_entire;

/* utility functions */
typedef union
{
    efidp_header header;
    efidp_hw_pci hw_pci;
    efidp_hw_pccard hw_pccard;
    efidp_hw_memory_mapped hw_memory_mapped;
    efidp_hw_vendor hw_vendor;
    efidp_hw_controller hw_controller;
    efidp_hw_bmc hw_bmc;
    efidp_acpi_acpi acpi_acpi;
    efidp_acpi_expanded acpi_expanded;
    efidp_acpi_adr acpi_adr;
    efidp_acpi_nvdimm acpi_nvdimm;
    efidp_msg_atapi msg_atapi;
    efidp_msg_scsi msg_scsi;
    efidp_msg_fibre_channel msg_fibre_channel;
    efidp_msg_firewire msg_firewire;
    efidp_msg_usb msg_usb;
    efidp_msg_i2o msg_i2o;
    efidp_msg_infiniband msg_infiniband;
    efidp_msg_vendor msg_vendor;
    efidp_msg_mac_address msg_mac_address;
    efidp_msg_ipv4 msg_ipv4;
    efidp_msg_ipv6 msg_ipv6;
    efidp_msg_uart msg_uart;
    efidp_msg_usb_class msg_usb_class;
    efidp_msg_usb_wwid msg_usb_wwid;
    efidp_msg_device_logical_unit msg_device_logical_unit;
    efidp_msg_sata msg_sata;
    efidp_msg_iscsi msg_iscsi;
    efidp_msg_vlan msg_vlan;
    efidp_msg_fibre_channel_ex msg_fibre_channel_ex;
    efidp_msg_sas_extended_messaging msg_sas_extended_messaging;
    efidp_msg_nvm_express_ns msg_nvm_express_ns;
    efidp_msg_uri msg_uri;
    efidp_msg_ufs msg_ufs;
    efidp_msg_sd msg_sd;
    efidp_msg_bluetooth msg_bluetooth;
    efidp_msg_wi_fi msg_wi_fi;
    efidp_msg_emmc msg_emmc;
    efidp_msg_bluetoothle msg_bluetoothle;
    efidp_msg_dns msg_dns;
    efidp_msg_nvdimm_ns msg_nvdimm_ns;
    efidp_msg_rest_service msg_rest_service;
    efidp_msg_nvme_of_ns msg_nvme_of_ns;
    efidp_media_hd media_hd;
    efidp_media_cd_rom media_cd_rom;
    efidp_media_vendor media_vendor;
    efidp_media_file_path media_file_path;
    efidp_media_protocol media_protocol;
    efidp_media_firmware_file media_firmware_file;
    efidp_media_firmware_volume media_firmware_volume;
    efidp_media_relative_offset_range media_relative_offset_range;
    efidp_media_ram_disk media_ram_disk;
    efidp_bios_boot_specification bios_boot_specification;
    efidp_end_instance end_instance;
    efidp_end_entire end_entire;
} efidp_data;
typedef efidp_data *efidp;
typedef const efidp_data *const_efidp;

#pragma pack(pop)

static inline int16_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED
    efidp_type(const_efidp dp)
{
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    return dp->header.type;
}

static inline int16_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED
    efidp_subtype(const_efidp dp)
{
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    return dp->header.subtype;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_WARN_UNUSED_RESULT
    efidp_node_size(const_efidp dn)
{
    if(ATTR_NONNULL_IS_NULL(dn) || dn->header.length < 4)
    {
        errno = EINVAL;
        return -1;
    }

    return dn->header.length;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_next_node(const_efidp in, const_efidp *out)
{
    if(efidp_type(in) == EFIDP_TYPE_END && efidp_subtype(in) == EFIDP_END_ENTIRE)
        return 0;

    ssize_t sz = efidp_node_size(in);
    if(sz < 0)
        return -1;

    *out = STATIC_CAST(const_efidp)(advance_bytes(in, STATIC_CAST(size_t)(sz)));
    if(*out < in)
    {
        errno = EINVAL;
        return -1;
    }

    return 1;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_next_instance(const_efidp in, const_efidp *out)
{
    if(efidp_type(in) != EFIDP_TYPE_END || efidp_subtype(in) != EFIDP_END_INSTANCE)
    {
        errno = EINVAL;
        return -1;
    }

    ssize_t sz = efidp_node_size(in);
    if(sz < 0)
        return -1;

    *out = STATIC_CAST(const_efidp)(advance_bytes(in, STATIC_CAST(size_t)(sz)));
    if(*out < in)
    {
        errno = EINVAL;
        return -1;
    }

    return 1;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_is_multiinstance(const_efidp dn)
{
    while(1)
    {
        const_efidp next = NULL;
        int rc = efidp_next_node(dn, &next);
        if(rc < 0)
        {
            errno = EINVAL;
            return -1;
        }
        else if(rc == 0)
            return 0;

        dn = next;
        if(efidp_type(dn) == EFIDP_TYPE_END && efidp_subtype(dn) == EFIDP_END_INSTANCE)
            return 1;

        if(efidp_type(dn) == EFIDP_TYPE_END && efidp_subtype(dn) == EFIDP_END_ENTIRE)
            return 0;
    }

    return 0;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_get_next_end(const_efidp in, const_efidp *out)
{
    while(1)
    {
        if(efidp_type(in) == EFIDP_TYPE_END)
        {
            *out = in;
            return 0;
        }

        ssize_t sz = efidp_node_size(in);
        if(sz < 0)
            break;

        const_efidp next = STATIC_CAST(const_efidp)(advance_bytes(in, STATIC_CAST(size_t)(sz)));
        if(next < in)
        {
            errno = EINVAL;
            return -1;
        }

        in = next;
    }

    return -1;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_size(const_efidp dp)
{
    ssize_t size = 0;
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    if(efidp_type(dp) == EFIDP_TYPE_END && efidp_subtype(dp) == EFIDP_END_ENTIRE)
        return efidp_node_size(dp);

    while(1)
    {
        ssize_t sz = efidp_node_size(dp);
        if(sz < 0)
            return sz;

        size += sz;
        const_efidp next = NULL;
        int rc = efidp_next_instance(dp, &next);
        if(rc < 0)
        {
            rc = efidp_next_node(dp, &next);
            if(rc < 0)
                return rc;

            if(rc == 0)
                break;
        }

        dp = next;
    }

    return size;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_instance_size(const_efidp dpi)
{
    ssize_t size = 0;
    while(1)
    {
        ssize_t sz = efidp_node_size(dpi);
        if(sz < 0)
            return sz;

        size += sz;
        if(efidp_type(dpi) == EFIDP_TYPE_END)
            break;

        const_efidp next = NULL;
        int rc = efidp_next_node(dpi, &next);
        if(rc < 0)
            return rc;

        dpi = next;
    }

    return size;
}
