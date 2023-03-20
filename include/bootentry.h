// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QHostAddress>
#include <QMetaType>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QVector>

#include "efiboot.h"

namespace File_path
{

template <class Type>
inline bool registerJSONReader();
#define REGISTER_JSON_READER(type) static const bool is_##type##_json_reader_registered = registerJSONReader<type>()

#if defined(_MSC_VER)
#pragma warning(push)
// C4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable : 4820)
#endif

class PCI
{
public:
    static constexpr auto TYPE = "HW";
    static constexpr auto SUBTYPE = "PCI";

private:
    mutable QString string = "";

public:
    uint8_t function = 0;
    uint8_t device = 0;

public:
    PCI() = default;
    PCI(const EFIBoot::File_path::PCI &pci);
    EFIBoot::File_path::PCI toEFIBootFilePath() const;

    static std::optional<PCI> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(PCI);

class HID
{
public:
    static constexpr auto TYPE = "ACPI";
    static constexpr auto SUBTYPE = "HID";

private:
    mutable QString string = "";

public:
    uint32_t hid = 0;
    uint32_t uid = 0;

public:
    HID() = default;
    HID(const EFIBoot::File_path::HID &hid);
    EFIBoot::File_path::HID toEFIBootFilePath() const;

    static std::optional<HID> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(HID);

class USB
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "USB";

private:
    mutable QString string = "";

public:
    uint8_t parent_port_number = 0;
    uint8_t interface = 0;

public:
    USB() = default;
    USB(const EFIBoot::File_path::USB &usb);
    EFIBoot::File_path::USB toEFIBootFilePath() const;

    static std::optional<USB> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(USB);

class Vendor
{
public:
    static constexpr auto TYPE = "MULTI";
    static constexpr auto SUBTYPE = "VENDOR";

private:
    mutable QString string = "";

public:
    uint8_t _type = 0;
    QUuid guid = {};
    QByteArray data = {};

public:
    Vendor() = default;
    Vendor(const EFIBoot::File_path::HWVendor &vendor);
    Vendor(const EFIBoot::File_path::MSGVendor &vendor);
    Vendor(const EFIBoot::File_path::MEDIAVendor &vendor);
    EFIBoot::File_path::ANY toEFIBootFilePath() const;

    static std::optional<Vendor> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(Vendor);

class MACAddress
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "MAC_ADDRESS";

private:
    mutable QString string = "";

public:
    QString address = "";
    uint8_t if_type = 0;

public:
    MACAddress() = default;
    MACAddress(const EFIBoot::File_path::MAC_address &mac_address);
    EFIBoot::File_path::MAC_address toEFIBootFilePath() const;

    static std::optional<MACAddress> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(MACAddress);

class IPv4
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "IPV4";

private:
    mutable QString string = "";

public:
    QHostAddress local_ip_address = {};
    QHostAddress remote_ip_address = {};
    uint16_t local_port = 0;
    uint16_t remote_port = 0;
    uint16_t protocol = 0;
    bool static_ip_address = false;
    QHostAddress gateway_ip_address = {};
    QHostAddress subnet_mask = {};

public:
    IPv4() = default;
    IPv4(const EFIBoot::File_path::IPv4 &ipv4);
    EFIBoot::File_path::IPv4 toEFIBootFilePath() const;

    static std::optional<IPv4> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(IPv4);

class IPv6
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "IPV6";

private:
    mutable QString string = "";

public:
    QHostAddress local_ip_address = {};
    QHostAddress remote_ip_address = {};
    uint16_t local_port = 0;
    uint16_t remote_port = 0;
    uint16_t protocol = 0;
    uint8_t ip_address_origin = 0;
    uint8_t prefix_length = 0;
    QHostAddress gateway_ip_address = {};

public:
    IPv6() = default;
    IPv6(const EFIBoot::File_path::IPv6 &ipv6);
    EFIBoot::File_path::IPv6 toEFIBootFilePath() const;

    static std::optional<IPv6> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(IPv6);

class SATA
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "SATA";

private:
    mutable QString string = "";

public:
    uint16_t hba_port = 0;
    uint16_t port_multiplier_port = 0;
    uint16_t lun = 0;

public:
    SATA() = default;
    SATA(const EFIBoot::File_path::SATA &sata);
    EFIBoot::File_path::SATA toEFIBootFilePath() const;

    static std::optional<SATA> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(SATA);

class HD
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "HD";

private:
    mutable QString string = "";

public:
    QUuid partition_signature = {};
    uint64_t partition_start = 0;
    uint64_t partition_size = 0;
    uint32_t partition_number = 0;
    uint8_t partition_format = 0;
    uint8_t signature_type = 9;

public:
    HD() = default;
    HD(const EFIBoot::File_path::HD &hd);
    EFIBoot::File_path::HD toEFIBootFilePath() const;

    static std::optional<HD> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(HD);

class File
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FILE";

public:
    QString name = "";

private:
    mutable QString string = "";

public:
    File() = default;
    File(const EFIBoot::File_path::File &file);
    EFIBoot::File_path::File toEFIBootFilePath() const;

    static std::optional<File> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(File);

class FirmwareFile
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FIRMWARE_FILE";

public:
    QUuid name = {};

private:
    mutable QString string = "";

public:
    FirmwareFile() = default;
    FirmwareFile(const EFIBoot::File_path::Firmware_file &firmware_file);
    EFIBoot::File_path::Firmware_file toEFIBootFilePath() const;

    static std::optional<FirmwareFile> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(FirmwareFile);

class FirmwareVolume
{
public:
    static constexpr auto TYPE = "MEDIA";
    static constexpr auto SUBTYPE = "FIRMWARE_VOLUME";

public:
    QUuid name = {};

private:
    mutable QString string = "";

public:
    FirmwareVolume() = default;
    FirmwareVolume(const EFIBoot::File_path::Firmware_volume &firmware_volume);
    EFIBoot::File_path::Firmware_volume toEFIBootFilePath() const;

    static std::optional<FirmwareVolume> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(FirmwareVolume);

class BIOSBootSpecification
{
public:
    static constexpr auto TYPE = "BIOS";
    static constexpr auto SUBTYPE = "BIOS_BOOT_SPECIFICATION";

public:
    uint16_t device_type = 0;
    uint16_t status_flag = 0;
    QString description = "";

private:
    mutable QString string = "";

public:
    BIOSBootSpecification() = default;
    BIOSBootSpecification(const EFIBoot::File_path::BIOS_boot_specification &bios_boot_specification);
    EFIBoot::File_path::BIOS_boot_specification toEFIBootFilePath() const;

    static std::optional<BIOSBootSpecification> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(BIOSBootSpecification);

class End
{
public:
    static constexpr auto TYPE = "END";
    static constexpr auto SUBTYPE = "MULTI";

private:
    mutable QString string = "";

public:
    EFIBoot::EFIDP_END _subtype = EFIBoot::EFIDP_END_ENTIRE;

public:
    End() = default;
    End(const EFIBoot::File_path::End_instance &)
        : _subtype{EFIBoot::EFIDP_END_INSTANCE}
    {
    }
    End(const EFIBoot::File_path::End_entire &)
        : _subtype{EFIBoot::EFIDP_END_ENTIRE}
    {
    }
    EFIBoot::File_path::ANY toEFIBootFilePath() const
    {
        switch(_subtype)
        {
        case EFIBoot::EFIDP_END_INSTANCE:
            return EFIBoot::File_path::End_instance{};
            break;

        case EFIBoot::EFIDP_END_ENTIRE:
            return EFIBoot::File_path::End_entire{};
            break;
        }

        return {};
    }

    static std::optional<End> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(End);

class Unknown
{
public:
    static constexpr auto TYPE = "UNK";
    static constexpr auto SUBTYPE = "UNKNOWN";

private:
    mutable QString string = "";

public:
    uint8_t _type = 0;
    uint8_t _subtype = 0;
    QByteArray data = {};

public:
    Unknown() = default;
    Unknown(const EFIBoot::File_path::Unknown &unknown);
    EFIBoot::File_path::Unknown toEFIBootFilePath() const;

    static std::optional<Unknown> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(Unknown);

typedef std::variant<
    PCI,
    HID,
    USB,
    Vendor,
    MACAddress,
    IPv4,
    IPv6,
    SATA,
    HD,
    File,
    FirmwareFile,
    FirmwareVolume,
    BIOSBootSpecification,
    End,
    Unknown>
    ANY;

extern std::unique_ptr<std::unordered_map<QString, std::function<std::optional<ANY>(const QJsonObject &)>>> JSON_readers__instance;

inline auto &JSON_readers()
{
    if(!JSON_readers__instance)
        JSON_readers__instance = std::make_unique<decltype(JSON_readers__instance)::element_type>();

    return *JSON_readers__instance;
}

template <class Type>
inline bool registerJSONReader()
{
    auto key = QString("%1/%2").arg(Type::TYPE).arg(Type::SUBTYPE);
    if(JSON_readers().find(key) != JSON_readers().end())
        return true;

    JSON_readers()[key] = [](const auto &obj) -> std::optional<ANY>
    { return Type::fromJSON(obj); };
    return true;
}

#undef REGISTER_JSON_READER
} // namespace File_path

Q_DECLARE_METATYPE(const File_path::ANY *)

class BootEntry
{
public:
    enum class OptionalDataFormat : uint8_t
    {
        Base64 = 0,
        Utf16 = 1,
        Utf8 = 2,
        Hex = 3,
    };

    uint16_t index = 0;
    QString description = "New entry";
    QVector<File_path::ANY> device_path = {};
    QString optional_data = "";
    uint32_t attributes = EFIBoot::Load_option_attribute::EMPTY;
    uint32_t efi_attributes = EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS;

    bool is_current_boot = false;
    bool is_next_boot = false;

    OptionalDataFormat optional_data_format = OptionalDataFormat::Base64;

private:
    mutable QString device_path_str = "";

public:
    static BootEntry fromEFIBootLoadOption(const EFIBoot::Load_option &load_option);
    EFIBoot::Load_option toEFIBootLoadOption() const;

    static std::optional<BootEntry> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString formatDevicePath(bool refresh = true) const;
    QString getTitle() const;

    bool changeOptionalDataFormat(OptionalDataFormat format, bool test = false);

private:
    QByteArray getRawOptionalData() const;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

Q_DECLARE_METATYPE(const BootEntry *)
