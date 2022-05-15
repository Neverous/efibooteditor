// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QHostAddress>
#include <QMetaType>
#include <QString>
#include <QUuid>
#include <QVariant>
#include <QVector>

#include "efiboot.h"

namespace Device_path
{

template <class Type>
inline bool register_json_reader();
#define REGISTER_JSON_READER(type) static const bool is_##type##_json_reader_registered = register_json_reader<type>()

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
    quint8 function = 0;
    quint8 device = 0;

public:
    PCI() = default;
    PCI(const EFIBoot::Device_path::PCI &pci);
    EFIBoot::Device_path::PCI toEFIBootDevicePath() const;

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
    quint32 hid = 0;
    quint32 uid = 0;

public:
    HID() = default;
    HID(const EFIBoot::Device_path::HID &hid);
    EFIBoot::Device_path::HID toEFIBootDevicePath() const;

    static std::optional<HID> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(HID);

class Vendor
{
public:
    static constexpr auto TYPE = "MSG";
    static constexpr auto SUBTYPE = "Vendor";

private:
    mutable QString string = "";

public:
    QUuid guid = {};
    QByteArray data = {};

public:
    Vendor() = default;
    Vendor(const EFIBoot::Device_path::Vendor &vendor);
    EFIBoot::Device_path::Vendor toEFIBootDevicePath() const;

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
    quint8 if_type = 0;

public:
    MACAddress() = default;
    MACAddress(const EFIBoot::Device_path::MAC_address &mac_address);
    EFIBoot::Device_path::MAC_address toEFIBootDevicePath() const;

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
    quint16 local_port = 0;
    quint16 remote_port = 0;
    quint16 protocol = 0;
    bool static_ip_address = false;
    QHostAddress gateway_ip_address = {};
    QHostAddress subnet_mask = {};

public:
    IPv4() = default;
    IPv4(const EFIBoot::Device_path::IPv4 &ipv4);
    EFIBoot::Device_path::IPv4 toEFIBootDevicePath() const;

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
    quint16 local_port = 0;
    quint16 remote_port = 0;
    quint16 protocol = 0;
    quint8 ip_address_origin = 0;
    quint8 prefix_length = 0;
    QHostAddress gateway_ip_address = {};

public:
    IPv6() = default;
    IPv6(const EFIBoot::Device_path::IPv6 &ipv6);
    EFIBoot::Device_path::IPv6 toEFIBootDevicePath() const;

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
    quint16 hba_port = 0;
    quint16 port_multiplier_port = 0;
    quint16 lun = 0;

public:
    SATA() = default;
    SATA(const EFIBoot::Device_path::SATA &sata);
    EFIBoot::Device_path::SATA toEFIBootDevicePath() const;

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
    quint64 partition_start = 0;
    quint64 partition_size = 0;
    quint32 partition_number = 0;
    quint8 partition_format = 0;
    quint8 signature_type = 9;

public:
    HD() = default;
    HD(const EFIBoot::Device_path::HD &hd);
    EFIBoot::Device_path::HD toEFIBootDevicePath() const;

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
    File(const EFIBoot::Device_path::File &file);
    EFIBoot::Device_path::File toEFIBootDevicePath() const;

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
    FirmwareFile(const EFIBoot::Device_path::Firmware_file &firmware_file);
    EFIBoot::Device_path::Firmware_file toEFIBootDevicePath() const;

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
    FirmwareVolume(const EFIBoot::Device_path::Firmware_volume &firmware_volume);
    EFIBoot::Device_path::Firmware_volume toEFIBootDevicePath() const;

    static std::optional<FirmwareVolume> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString toString(bool refresh = true) const;
};
REGISTER_JSON_READER(FirmwareVolume);

typedef std::variant<
    PCI,
    HID,
    Vendor,
    MACAddress,
    IPv4,
    IPv6,
    SATA,
    HD,
    File,
    FirmwareFile,
    FirmwareVolume>
    ANY;

extern std::unique_ptr<std::unordered_map<QString, std::function<std::optional<ANY>(const QJsonObject &)>>> JSON_readers__instance;

inline auto &JSON_readers()
{
    if(!JSON_readers__instance)
        JSON_readers__instance = std::make_unique<decltype(JSON_readers__instance)::element_type>();

    return *JSON_readers__instance;
}

template <class Type>
inline bool register_json_reader()
{
    auto key = QString("%1/%2").arg(Type::TYPE).arg(Type::SUBTYPE);
    if(JSON_readers().find(key) != JSON_readers().end())
        return true;

    JSON_readers()[key] = [](const auto &obj) -> std::optional<ANY>
    { return Type::fromJSON(obj); };
    return true;
}

#undef REGISTER_JSON_READER
} // namespace Device_path

Q_DECLARE_METATYPE(const Device_path::ANY *)

class BootEntry
{
public:
    enum OptionalDataFormat
    {
        Base64 = 0,
        Utf16 = 1,
        Utf8 = 2,
        Hex = 3,
    };

    quint16 index = 0;
    QString description = "New entry";
    QVector<Device_path::ANY> file_path = {};
    QString optional_data = "";
    quint32 attributes = 0;
    quint32 efi_attributes = EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS;

    bool is_next_boot = false;

    OptionalDataFormat optional_data_format = OptionalDataFormat::Base64;

private:
    mutable QString file_path_str = "";

public:
    static BootEntry fromEFIBootLoadOption(const EFIBoot::Load_option &load_option);
    EFIBoot::Load_option toEFIBootLoadOption() const;

    static std::optional<BootEntry> fromJSON(const QJsonObject &obj);
    QJsonObject toJSON() const;

    QString format_file_path(bool refresh = true) const;

    bool change_optional_data_format(OptionalDataFormat format);

private:
    QByteArray get_raw_optional_data() const;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

Q_DECLARE_METATYPE(const BootEntry *)
