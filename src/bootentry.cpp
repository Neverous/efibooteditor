// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentry.h"

#include "compat.h"
#include "efiboot.h"
#include <QJsonArray>
#include <QJsonObject>

EFIBoot::Progress_fn EFIBoot::_get_variables_progress_fn = nullptr;

#define check_obj()                                      \
    if(obj["type"] != TYPE || obj["subtype"] != SUBTYPE) \
    return std::nullopt
#define check_type(field, typecheck)                          \
    if(!obj.contains(#field) || !obj[#field].is##typecheck()) \
    return std::nullopt
#define try_read(field, typecheck) try_read_3(field, typecheck, typecheck)
#define try_read_3(field, typecheck, typecast) \
    check_type(field, typecheck);              \
    value.field = static_cast<decltype(value.field)>(obj[#field].to##typecast())

auto BootEntry::fromEFIBootLoadOption(
    const EFIBoot::Load_option &load_option) -> BootEntry
{
    BootEntry value{};
    value.description = QString::fromStdU16String(load_option.description);

    value.optional_data_format = OptionalDataFormat::Base64;
    if(toUnicode(value.optional_data, load_option.optional_data, "UTF-8") && !value.optional_data.contains(QChar(0)))
        value.optional_data_format = OptionalDataFormat::Utf8;

    if(value.optional_data_format == OptionalDataFormat::Base64 && load_option.optional_data.size() % sizeof(char16_t) == 0 && toUnicode(value.optional_data, load_option.optional_data, "UTF-16") && !value.optional_data.contains(QChar(0)))
        value.optional_data_format = OptionalDataFormat::Utf16;

    if(value.optional_data_format == OptionalDataFormat::Base64)
        value.optional_data = QByteArray::fromRawData(reinterpret_cast<const char *>(load_option.optional_data.data()), static_cast<int>(load_option.optional_data.size())).toBase64();

    value.attributes = load_option.attributes;

    for(const auto &file_path: load_option.device_path)
        value.device_path.push_back(std::visit([](const auto &path) -> File_path::ANY
            { return path; },
            file_path));

    return value;
}

auto BootEntry::fromError(const QString &error) -> BootEntry
{
    BootEntry value{};
    value.is_error = true;
    value.description = "Error";
    value.error = error;
    return value;
}

auto BootEntry::toEFIBootLoadOption() const -> EFIBoot::Load_option
{
    if(is_error)
        return {};

    EFIBoot::Load_option load_option{};
    load_option.description = description.toStdU16String();
    {
        auto bytes = getRawOptionalData();
        auto begin = reinterpret_cast<const EFIBoot::Raw_data::value_type *>(bytes.constData());
        std::copy(begin, std::next(begin, bytes.size()), std::back_inserter(load_option.optional_data));
    }

    load_option.attributes = attributes;
    for(const auto &file_path: device_path)
        load_option.device_path.push_back(std::visit([](const auto &obj) -> EFIBoot::File_path::ANY
            { return obj.toEFIBootFilePath(); },
            file_path));

    return load_option;
}

auto BootEntry::fromJSON(const QJsonObject &obj) -> std::optional<BootEntry>
{
    BootEntry value{};
    try_read(description, String);
    try_read_3(optional_data_format, Double, Int);
    try_read(optional_data, String);
    try_read_3(attributes, Double, Int);
    try_read_3(efi_attributes, Double, Int);
    check_type(file_path, Array);
    const auto device_path = obj["file_path"].toArray();
    for(const auto file_path: device_path)
    {
        auto dp = file_path.toObject();
        auto path = get_default(File_path::JSON_readers(), QString("%1/%2").arg(dp["type"].toString(), dp["subtype"].toString()), [](const auto &)
            { return std::nullopt; })(dp);
        if(!path)
            return std::nullopt;

        value.device_path.push_back(*path);
    }

    return {value};
}

auto BootEntry::toJSON() const -> QJsonObject
{
    if(is_error)
        return {};

    QJsonObject load_option;
    load_option["description"] = description;
    load_option["optional_data_format"] = static_cast<int>(optional_data_format);
    load_option["optional_data"] = optional_data;
    load_option["attributes"] = static_cast<int>(attributes);
    load_option["efi_attributes"] = static_cast<int>(efi_attributes);
    QJsonArray file_path_json;
    for(const auto &file_path: device_path)
        file_path_json.push_back(std::visit([](const auto &obj) -> QJsonObject
            { return obj.toJSON(); },
            file_path));

    load_option["file_path"] = file_path_json;
    return load_option;
}

auto BootEntry::formatDevicePath(bool refresh) const -> QString
{
    if(device_path.empty())
        return {};

    if(device_path_str.size() && !refresh)
        return device_path_str;

    device_path_str.clear();
    for(const auto &file_path: device_path)
    {
        if(!device_path_str.isEmpty())
            device_path_str += "/";

        device_path_str += std::visit([refresh](const auto &obj)
            { return obj.toString(refresh); },
            file_path);
    }

    return device_path_str;
}

QString BootEntry::getTitle() const
{
    return QString("%1 (%2)").arg(description, toHex(index, 4));
}

auto BootEntry::changeOptionalDataFormat(BootEntry::OptionalDataFormat format, bool test) -> bool
{
    if(format == optional_data_format)
        return true;

    auto bytes = getRawOptionalData();
    QString temp_optional_data;
    switch(format)
    {
    case OptionalDataFormat::Base64:
        temp_optional_data = bytes.toBase64();
        break;

    case OptionalDataFormat::Utf16:
        if(static_cast<uint>(bytes.size()) % sizeof(char16_t) != 0)
            return false;

        if(!toUnicode(temp_optional_data, bytes, "UTF-16"))
            return false;

        break;

    case OptionalDataFormat::Utf8:
        if(!toUnicode(temp_optional_data, bytes, "UTF-8"))
            return false;

        break;

    case OptionalDataFormat::Hex:
        temp_optional_data = bytes.toHex();
        break;
    }

    if(temp_optional_data.contains(QChar(0)))
        return false;

    if(!test)
    {
        optional_data_format = format;
        optional_data = temp_optional_data;
    }
    return true;
}

auto BootEntry::getRawOptionalData() const -> QByteArray
{
    QByteArray bytes;
    switch(optional_data_format)
    {
    case OptionalDataFormat::Base64:
        bytes = QByteArray::fromBase64(optional_data.toUtf8());
        break;

    case OptionalDataFormat::Utf16:
        bytes = fromUnicode(optional_data, "UTF-16");
        break;

    case OptionalDataFormat::Utf8:
        bytes = fromUnicode(optional_data, "UTF-8");
        break;

    case OptionalDataFormat::Hex:
        bytes = QByteArray::fromHex(optional_data.toUtf8());
        break;
    }

    return bytes;
}

File_path::PCI::PCI(const EFIBoot::File_path::PCI &pci)
    : function{pci.function}
    , device{pci.device}
{
}

auto File_path::PCI::toEFIBootFilePath() const -> EFIBoot::File_path::PCI
{
    EFIBoot::File_path::PCI value = {};
    value.function = function;
    value.device = device;
    return value;
}

auto File_path::PCI::fromJSON(const QJsonObject &obj) -> std::optional<File_path::PCI>
{
    PCI value{};
    check_obj();
    try_read_3(function, Double, Int);
    try_read_3(device, Double, Int);
    return {value};
}

auto File_path::PCI::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["function"] = static_cast<int>(function);
    value["device"] = static_cast<int>(device);
    return value;
}

auto File_path::PCI::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Pci(%1,%2)").arg(device).arg(function);
}

File_path::HID::HID(const EFIBoot::File_path::HID &_hid)
    : hid{_hid.hid}
    , uid{_hid.uid}
{
}

auto File_path::HID::toEFIBootFilePath() const -> EFIBoot::File_path::HID
{
    EFIBoot::File_path::HID value = {};
    value.hid = hid;
    value.uid = uid;
    return value;
}

auto File_path::HID::fromJSON(const QJsonObject &obj) -> std::optional<File_path::HID>
{
    HID value{};
    check_obj();
    try_read_3(hid, Double, Int);
    try_read_3(uid, Double, Int);
    return {value};
}

auto File_path::HID::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hid"] = static_cast<int>(hid);
    value["uid"] = static_cast<int>(uid);
    return value;
}

auto File_path::HID::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Acpi(%1,%2)").arg(toHex(hid), toHex(uid));
}

File_path::USB::USB(const EFIBoot::File_path::USB &usb)
    : parent_port_number{usb.parent_port_number}
    , interface {
    usb.interface
}
{
}

auto File_path::USB::toEFIBootFilePath() const -> EFIBoot::File_path::USB
{
    EFIBoot::File_path::USB value = {};
    value.parent_port_number = parent_port_number;
    value.interface = interface;
    return value;
}

auto File_path::USB::fromJSON(const QJsonObject &obj) -> std::optional<File_path::USB>
{
    USB value{};
    check_obj();
    try_read_3(parent_port_number, Double, Int);
    try_read_3(interface, Double, Int);
    return {value};
}

auto File_path::USB::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["parent_port_number"] = static_cast<int>(parent_port_number);
    value["interface"] = static_cast<int>(interface);
    return value;
}

auto File_path::USB::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("USB(%1,%2)").arg(parent_port_number).arg(interface);
}

static_assert(sizeof(File_path::Vendor::guid) == sizeof(EFIBoot::File_path::HWVendor::guid));
static_assert(sizeof(File_path::Vendor::guid) == sizeof(EFIBoot::File_path::MSGVendor::guid));
static_assert(sizeof(File_path::Vendor::guid) == sizeof(EFIBoot::File_path::MEDIAVendor::guid));

File_path::Vendor::Vendor(const EFIBoot::File_path::HWVendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size()))}
    , _type{EFIBoot::File_path::HWVendor::TYPE}
{
    data.detach();
    static_assert(sizeof(vendor.guid) == sizeof(guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

File_path::Vendor::Vendor(const EFIBoot::File_path::MSGVendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size()))}
    , _type{EFIBoot::File_path::MSGVendor::TYPE}
{
    data.detach();
    static_assert(sizeof(vendor.guid) == sizeof(guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

File_path::Vendor::Vendor(const EFIBoot::File_path::MEDIAVendor &vendor)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size()))}
    , _type{EFIBoot::File_path::MEDIAVendor::TYPE}
{
    data.detach();
    static_assert(sizeof(vendor.guid) == sizeof(guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

auto File_path::Vendor::toEFIBootFilePath() const -> EFIBoot::File_path::ANY
{
    switch(_type)
    {
    case EFIBoot::File_path::HWVendor::TYPE:
    {
        EFIBoot::File_path::HWVendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    case EFIBoot::File_path::MSGVendor::TYPE:
    {
        EFIBoot::File_path::MSGVendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    case EFIBoot::File_path::MEDIAVendor::TYPE:
    {
        EFIBoot::File_path::MEDIAVendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }

    default:
        return {};
    }
}

auto File_path::Vendor::fromJSON(const QJsonObject &obj) -> std::optional<File_path::Vendor>
{
    Vendor value{};
    check_obj();
    try_read_3(_type, Double, Int);
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto File_path::Vendor::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["_type"] = _type;
    value["guid"] = guid.toString();
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto File_path::Vendor::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    const char *type_string = nullptr;
    switch(_type)
    {
    case EFIBoot::File_path::HWVendor::TYPE:
        type_string = "Hw";
        break;

    case EFIBoot::File_path::MSGVendor::TYPE:
        type_string = "Msg";
        break;

    case EFIBoot::File_path::MEDIAVendor::TYPE:
        type_string = "Media";
        break;

    default:
        type_string = "Unk";
        break;
    }

    return string = QString("Ven%1(%2,[%3B])").arg(type_string, guid.toString(QUuid::WithoutBraces)).arg(data.size());
}

File_path::MACAddress::MACAddress(const EFIBoot::File_path::MAC_address &mac_address)
    : address{QByteArray::fromRawData(reinterpret_cast<const char *>(mac_address.address.data()), static_cast<int>(mac_address.address.size())).toHex()}
    , if_type{mac_address.if_type}
{
}

auto File_path::MACAddress::toEFIBootFilePath() const -> EFIBoot::File_path::MAC_address
{
    EFIBoot::File_path::MAC_address value = {};
    auto address_bytes = QByteArray::fromHex(address.toUtf8());
    memcpy(value.address.data(), address_bytes.data(), value.address.size());
    value.if_type = if_type;
    return value;
}

auto File_path::MACAddress::fromJSON(const QJsonObject &obj) -> std::optional<File_path::MACAddress>
{
    MACAddress value{};
    check_obj();
    try_read(address, String);
    try_read_3(if_type, Double, Int);
    return {value};
}

auto File_path::MACAddress::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["address"] = address;
    value["if_type"] = if_type;
    return value;
}

auto File_path::MACAddress::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("MAC(%1,%2)").arg(address.left(12)).arg(if_type);
}

static_assert(sizeof(File_path::IPv4::local_ip_address.toIPv4Address()) == sizeof(EFIBoot::File_path::IPv4::local_ip_address));
static_assert(sizeof(File_path::IPv4::remote_ip_address.toIPv4Address()) == sizeof(EFIBoot::File_path::IPv4::remote_ip_address));
static_assert(sizeof(File_path::IPv4::gateway_ip_address.toIPv4Address()) == sizeof(EFIBoot::File_path::IPv4::gateway_ip_address));
static_assert(sizeof(File_path::IPv4::subnet_mask.toIPv4Address()) == sizeof(EFIBoot::File_path::IPv4::subnet_mask));

File_path::IPv4::IPv4(const EFIBoot::File_path::IPv4 &ipv4)
    : local_ip_address{*reinterpret_cast<const uint32_t *>(ipv4.local_ip_address.data())}
    , remote_ip_address{*reinterpret_cast<const uint32_t *>(ipv4.remote_ip_address.data())}
    , gateway_ip_address{*reinterpret_cast<const uint32_t *>(ipv4.gateway_ip_address.data())}
    , subnet_mask{*reinterpret_cast<const uint32_t *>(ipv4.subnet_mask.data())}
    , local_port{ipv4.local_port}
    , remote_port{ipv4.remote_port}
    , protocol{ipv4.protocol}
    , static_ip_address{ipv4.static_ip_address}
{
}

auto File_path::IPv4::toEFIBootFilePath() const -> EFIBoot::File_path::IPv4
{
    EFIBoot::File_path::IPv4 value = {};
    auto ip_address = local_ip_address.toIPv4Address();
    static_assert(sizeof(ip_address) == sizeof(value.local_ip_address));
    memcpy(value.local_ip_address.data(), &ip_address, sizeof(ip_address));
    ip_address = remote_ip_address.toIPv4Address();
    static_assert(sizeof(ip_address) == sizeof(value.remote_ip_address));
    memcpy(value.remote_ip_address.data(), &ip_address, sizeof(ip_address));
    value.local_port = local_port;
    value.remote_port = remote_port;
    value.protocol = protocol;
    value.static_ip_address = static_ip_address;
    ip_address = gateway_ip_address.toIPv4Address();
    static_assert(sizeof(ip_address) == sizeof(value.gateway_ip_address));
    memcpy(value.gateway_ip_address.data(), &ip_address, sizeof(ip_address));
    ip_address = subnet_mask.toIPv4Address();
    static_assert(sizeof(ip_address) == sizeof(value.subnet_mask));
    memcpy(value.subnet_mask.data(), &ip_address, sizeof(ip_address));
    return value;
}

auto File_path::IPv4::fromJSON(const QJsonObject &obj) -> std::optional<File_path::IPv4>
{
    IPv4 value{};
    check_obj();
    try_read(local_ip_address, String);
    try_read(remote_ip_address, String);
    try_read_3(local_port, Double, Int);
    try_read_3(remote_port, Double, Int);
    try_read_3(protocol, Double, Int);
    try_read(static_ip_address, Bool);
    try_read(gateway_ip_address, String);
    try_read(subnet_mask, String);
    return {value};
}

auto File_path::IPv4::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["local_ip_address"] = local_ip_address.toString();
    value["remote_ip_address"] = remote_ip_address.toString();
    value["local_port"] = local_port;
    value["remote_port"] = remote_port;
    value["protocol"] = protocol;
    value["static_ip_address"] = static_ip_address;
    value["gateway_ip_address"] = gateway_ip_address.toString();
    value["subnet_mask"] = subnet_mask.toString();
    return value;
}

auto File_path::IPv4::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("IPv4(%1:%2,%3,%4,%5:%6,%7,%8)").arg(remote_ip_address.toString()).arg(remote_port).arg(protocol).arg(static_ip_address ? "Static" : "DHCP", local_ip_address.toString()).arg(local_port).arg(gateway_ip_address.toString(), subnet_mask.toString());
}

static_assert(sizeof(File_path::IPv6::local_ip_address.toIPv6Address()) == sizeof(EFIBoot::File_path::IPv6::local_ip_address));
static_assert(sizeof(File_path::IPv6::remote_ip_address.toIPv6Address()) == sizeof(EFIBoot::File_path::IPv6::remote_ip_address));
static_assert(sizeof(File_path::IPv6::gateway_ip_address.toIPv6Address()) == sizeof(EFIBoot::File_path::IPv6::gateway_ip_address));

File_path::IPv6::IPv6(const EFIBoot::File_path::IPv6 &ipv6)
    : local_ip_address{ipv6.local_ip_address.data()}
    , remote_ip_address{ipv6.remote_ip_address.data()}
    , local_port{ipv6.local_port}
    , remote_port{ipv6.remote_port}
    , protocol{ipv6.protocol}
    , ip_address_origin{ipv6.ip_address_origin}
    , prefix_length{ipv6.prefix_length}
    , gateway_ip_address{ipv6.gateway_ip_address.data()}
{
}

auto File_path::IPv6::toEFIBootFilePath() const -> EFIBoot::File_path::IPv6
{
    EFIBoot::File_path::IPv6 value = {};
    auto ip_address = local_ip_address.toIPv6Address();
    static_assert(sizeof(ip_address) == sizeof(value.local_ip_address));
    memcpy(value.local_ip_address.data(), &ip_address, sizeof(ip_address));
    ip_address = remote_ip_address.toIPv6Address();
    static_assert(sizeof(ip_address) == sizeof(value.remote_ip_address));
    memcpy(value.remote_ip_address.data(), &ip_address, sizeof(ip_address));
    value.local_port = local_port;
    value.remote_port = remote_port;
    value.protocol = protocol;
    value.ip_address_origin = ip_address_origin;
    value.prefix_length = prefix_length;
    ip_address = gateway_ip_address.toIPv6Address();
    static_assert(sizeof(ip_address) == sizeof(value.gateway_ip_address));
    memcpy(value.gateway_ip_address.data(), &ip_address, sizeof(ip_address));
    return value;
}

auto File_path::IPv6::fromJSON(const QJsonObject &obj) -> std::optional<File_path::IPv6>
{
    IPv6 value{};
    check_obj();
    try_read(local_ip_address, String);
    try_read(remote_ip_address, String);
    try_read_3(local_port, Double, Int);
    try_read_3(remote_port, Double, Int);
    try_read_3(protocol, Double, Int);
    try_read_3(ip_address_origin, Double, Int);
    try_read_3(prefix_length, Double, Int);
    try_read(gateway_ip_address, String);
    return {value};
}

auto File_path::IPv6::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["local_ip_address"] = local_ip_address.toString();
    value["remote_ip_address"] = remote_ip_address.toString();
    value["local_port"] = local_port;
    value["remote_port"] = remote_port;
    value["protocol"] = protocol;
    value["ip_address_origin"] = ip_address_origin;
    value["prefix_length"] = prefix_length;
    value["gateway_ip_address"] = gateway_ip_address.toString();
    return value;
}

auto File_path::IPv6::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    QString origin;
    switch(ip_address_origin)
    {
    case 0:
        origin = "Static";
        break;
    case 1:
        origin = "StatelessAutoConfigure";
        break;
    case 2:
        origin = "StatefulAutoConfigure";
        break;
    default:
        origin = QString::number(ip_address_origin);
        break;
    }

    return string = QString("IPv6(%1:%2,%3,%4,%5:%6,%7,%8)").arg(remote_ip_address.toString()).arg(remote_port).arg(protocol).arg(origin, local_ip_address.toString()).arg(local_port).arg(gateway_ip_address.toString()).arg(prefix_length);
}

File_path::SATA::SATA(const EFIBoot::File_path::SATA &sata)
    : hba_port{sata.hba_port}
    , port_multiplier_port{sata.port_multiplier_port}
    , lun{sata.lun}
{
}

auto File_path::SATA::toEFIBootFilePath() const -> EFIBoot::File_path::SATA
{
    EFIBoot::File_path::SATA value = {};
    value.hba_port = hba_port;
    value.port_multiplier_port = port_multiplier_port;
    value.lun = lun;
    return value;
}

auto File_path::SATA::fromJSON(const QJsonObject &obj) -> std::optional<File_path::SATA>
{
    SATA value{};
    check_obj();
    try_read_3(hba_port, Double, Int);
    try_read_3(port_multiplier_port, Double, Int);
    try_read_3(lun, Double, Int);
    return {value};
}

auto File_path::SATA::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hba_port"] = static_cast<int>(hba_port);
    value["port_multiplier_port"] = static_cast<int>(port_multiplier_port);
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto File_path::SATA::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Sata(%1,%2,%3)").arg(hba_port).arg(port_multiplier_port).arg(lun);
}

File_path::URI::URI(const EFIBoot::File_path::URI &_uri)
    : uri{QUrl::fromEncoded(QString::fromStdString(_uri.uri).toUtf8())}
{
}

auto File_path::URI::toEFIBootFilePath() const -> EFIBoot::File_path::URI
{
    EFIBoot::File_path::URI value = {};
    value.uri = uri.toEncoded().toStdString();
    return value;
}

auto File_path::URI::fromJSON(const QJsonObject &obj) -> std::optional<File_path::URI>
{
    URI value{};
    check_obj();
    value.uri = QUrl::fromEncoded(obj["uri"].toString().toUtf8());
    return {value};
}

auto File_path::URI::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["uri"] = QString(uri.toEncoded());
    return value;
}

auto File_path::URI::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Uri(%1)").arg(uri.toDisplayString());
}

static_assert(sizeof(File_path::HD::partition_signature) == sizeof(EFIBoot::File_path::HD::partition_signature));

File_path::HD::HD(const EFIBoot::File_path::HD &hd)
    : partition_start{hd.partition_start}
    , partition_size{hd.partition_size}
    , partition_number{hd.partition_number}
    , partition_format{hd.partition_format}
    , signature_type{hd.signature_type}
{
    static_assert(sizeof(hd.partition_signature) == sizeof(partition_signature));
    memcpy(reinterpret_cast<void *>(&partition_signature), &hd.partition_signature, sizeof(hd.partition_signature));
}

auto File_path::HD::toEFIBootFilePath() const -> EFIBoot::File_path::HD
{
    EFIBoot::File_path::HD hd{};
    hd.partition_start = partition_start;
    hd.partition_size = partition_size;
    hd.partition_number = partition_number;
    hd.partition_format = partition_format;
    static_assert(sizeof(partition_signature) == sizeof(hd.partition_signature));
    memcpy(hd.partition_signature.data(), &partition_signature, sizeof(partition_signature));
    hd.signature_type = signature_type;
    return hd;
}

auto File_path::HD::fromJSON(const QJsonObject &obj) -> std::optional<File_path::HD>
{
    HD value{};
    check_obj();
    check_type(partition_start, String);
    value.partition_start = obj["partition_start"].toString().toULongLong(nullptr, HEX_BASE);
    check_type(partition_size, String);
    value.partition_size = obj["partition_size"].toString().toULongLong(nullptr, HEX_BASE);
    try_read_3(partition_number, Double, Int);
    try_read_3(partition_format, Double, Int);
    check_type(partition_signature, String);
    value.partition_signature = QUuid::fromString(obj["partition_signature"].toString());
    try_read_3(signature_type, Double, Int);
    return {value};
}

auto File_path::HD::toJSON() const -> QJsonObject
{
    QJsonObject hd{};
    hd["type"] = TYPE;
    hd["subtype"] = SUBTYPE;
    hd["partition_start"] = toHex(partition_start);
    hd["partition_size"] = toHex(partition_size);
    hd["partition_number"] = static_cast<int>(partition_number);
    hd["partition_format"] = static_cast<int>(partition_format);
    hd["partition_signature"] = partition_signature.toString();
    hd["signature_type"] = static_cast<int>(signature_type);
    return hd;
}

auto File_path::HD::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    QString format = QString("HD(%1,%2,%3,%4,%5)").arg(partition_number);
    switch(signature_type)
    {
    case EFIBoot::File_path::HD::SIGNATURE::MBR:
    {
        format = format.arg("MBR", toHex(partition_signature.data1));
        break;
    }

    case EFIBoot::File_path::HD::SIGNATURE::GUID:
        format = format.arg("GPT", partition_signature.toString(QUuid::WithoutBraces));
        break;

    case EFIBoot::File_path::HD::SIGNATURE::NONE:
        format = format.arg(static_cast<std::underlying_type_t<decltype(signature_type)>>(signature_type)).arg("N/A");
        break;
    }

    return string = format.arg(toHex(partition_start), toHex(partition_size));
}

File_path::File::File(const EFIBoot::File_path::File &file)
    : name{QString::fromStdU16String(file.name)}
{
}

auto File_path::File::toEFIBootFilePath() const -> EFIBoot::File_path::File
{
    EFIBoot::File_path::File file{};
    file.name = name.toStdU16String();
    return file;
}

auto File_path::File::fromJSON(const QJsonObject &obj) -> std::optional<File_path::File>
{
    File value{};
    check_obj();
    try_read(name, String);
    return {value};
}

auto File_path::File::toJSON() const -> QJsonObject
{
    QJsonObject file{};
    file["type"] = TYPE;
    file["subtype"] = SUBTYPE;
    file["name"] = name;
    return file;
}

auto File_path::File::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("File(%1)").arg(name);
}

static_assert(sizeof(File_path::FirmwareFile::name) == sizeof(EFIBoot::File_path::Firmware_file::name));

File_path::FirmwareFile::FirmwareFile(const EFIBoot::File_path::Firmware_file &firmware_file)
{
    static_assert(sizeof(firmware_file.name) == sizeof(name));
    memcpy(reinterpret_cast<void *>(&name), &firmware_file.name, sizeof(firmware_file.name));
}

auto File_path::FirmwareFile::toEFIBootFilePath() const -> EFIBoot::File_path::Firmware_file
{
    EFIBoot::File_path::Firmware_file firmware_file{};
    static_assert(sizeof(name) == sizeof(firmware_file.name));
    memcpy(firmware_file.name.data(), &name, sizeof(name));
    return firmware_file;
}

auto File_path::FirmwareFile::fromJSON(const QJsonObject &obj) -> std::optional<File_path::FirmwareFile>
{
    FirmwareFile value{};
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto File_path::FirmwareFile::toJSON() const -> QJsonObject
{
    QJsonObject firmware_file{};
    firmware_file["type"] = TYPE;
    firmware_file["subtype"] = SUBTYPE;
    firmware_file["name"] = name.toString();
    return firmware_file;
}

auto File_path::FirmwareFile::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("FvFile(%1)").arg(name.toString(QUuid::WithoutBraces));
}

static_assert(sizeof(File_path::FirmwareVolume::name) == sizeof(EFIBoot::File_path::Firmware_volume::name));

File_path::FirmwareVolume::FirmwareVolume(const EFIBoot::File_path::Firmware_volume &firmware_volume)
{
    static_assert(sizeof(firmware_volume.name) == sizeof(name));
    memcpy(reinterpret_cast<void *>(&name), &firmware_volume.name, sizeof(firmware_volume.name));
}

auto File_path::FirmwareVolume::toEFIBootFilePath() const -> EFIBoot::File_path::Firmware_volume
{
    EFIBoot::File_path::Firmware_volume firmware_volume{};
    static_assert(sizeof(name) == sizeof(firmware_volume.name));
    memcpy(firmware_volume.name.data(), &name, sizeof(name));
    return firmware_volume;
}

auto File_path::FirmwareVolume::fromJSON(const QJsonObject &obj) -> std::optional<File_path::FirmwareVolume>
{
    FirmwareVolume value{};
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto File_path::FirmwareVolume::toJSON() const -> QJsonObject
{
    QJsonObject firmware_volume{};
    firmware_volume["type"] = TYPE;
    firmware_volume["subtype"] = SUBTYPE;
    firmware_volume["name"] = name.toString();
    return firmware_volume;
}

auto File_path::FirmwareVolume::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Fv(%1)").arg(name.toString(QUuid::WithoutBraces));
}

File_path::BIOSBootSpecification::BIOSBootSpecification(const EFIBoot::File_path::BIOS_boot_specification &bios_boot_specification)
    : description{QString::fromStdString(bios_boot_specification.description)}
    , device_type{bios_boot_specification.device_type}
    , status_flag{bios_boot_specification.status_flag}
{
}

auto File_path::BIOSBootSpecification::toEFIBootFilePath() const -> EFIBoot::File_path::BIOS_boot_specification
{
    EFIBoot::File_path::BIOS_boot_specification value = {};
    value.device_type = device_type;
    value.status_flag = status_flag;
    value.description = description.toStdString();
    return value;
}

auto File_path::BIOSBootSpecification::fromJSON(const QJsonObject &obj) -> std::optional<File_path::BIOSBootSpecification>
{
    BIOSBootSpecification value{};
    check_obj();
    try_read_3(device_type, Double, Int);
    try_read_3(status_flag, Double, Int);
    try_read(description, String);
    return {value};
}

auto File_path::BIOSBootSpecification::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["device_type"] = static_cast<int>(device_type);
    value["status_flag"] = static_cast<int>(status_flag);
    value["description"] = description;
    return value;
}

auto File_path::BIOSBootSpecification::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("BBS(%1,%2,%3)").arg(toHex(device_type), description, toHex(status_flag));
}

auto File_path::End::fromJSON(const QJsonObject &obj) -> std::optional<File_path::End>
{
    End value{};
    check_obj();
    try_read_3(_subtype, Double, Int);
    return {value};
}

auto File_path::End::toJSON() const -> QJsonObject
{
    QJsonObject end_instance;
    end_instance["type"] = TYPE;
    end_instance["subtype"] = SUBTYPE;
    end_instance["_subtype"] = _subtype;
    return end_instance;
}

auto File_path::End::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    const char *subtype_string = "Unknown";
    switch(_subtype)
    {
    case EFIBoot::File_path::End_instance::SUBTYPE:
        subtype_string = "Instance";
        break;

    case EFIBoot::File_path::End_entire::SUBTYPE:
        subtype_string = "Entire";
        break;
    }

    return string = QString("End(%1)").arg(subtype_string);
}

File_path::Unknown::Unknown(const EFIBoot::File_path::Unknown &unknown)
    : data{QByteArray::fromRawData(reinterpret_cast<const char *>(unknown.data.data()), static_cast<int>(unknown.data.size()))}
    , _type{unknown.TYPE}
    , _subtype{unknown.SUBTYPE}
{
    data.detach();
}

auto File_path::Unknown::toEFIBootFilePath() const -> EFIBoot::File_path::Unknown
{
    EFIBoot::File_path::Unknown value = {};
    value.TYPE = _type;
    value.SUBTYPE = _subtype;
    value.data.resize(static_cast<size_t>(data.size()));
    std::copy(std::begin(data), std::end(data), std::begin(value.data));
    return value;
}

auto File_path::Unknown::fromJSON(const QJsonObject &obj) -> std::optional<File_path::Unknown>
{
    Unknown value{};
    check_obj();
    try_read_3(_type, Double, Int);
    try_read_3(_subtype, Double, Int);
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto File_path::Unknown::toJSON() const -> QJsonObject
{
    QJsonObject value{};
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["_type"] = _type;
    value["_subtype"] = _subtype;
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto File_path::Unknown::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Unknown(%1,%2,[%3B])").arg(toHex(_type), toHex(_subtype)).arg(data.size());
}

#undef try_read_3
#undef try_read
#undef check_type
#undef check_obj
