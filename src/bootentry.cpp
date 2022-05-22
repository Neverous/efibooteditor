// SPDX-License-Identifier: LGPL-3.0-or-later
#include "bootentry.h"

#include "efiboot.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QTextCodec>

std::unique_ptr<std::unordered_map<QString, std::function<std::optional<Device_path::ANY>(const QJsonObject &)>>> Device_path::JSON_readers__instance;
std::unique_ptr<std::unordered_map<uint16_t, std::function<std::optional<EFIBoot::Device_path::ANY>(const void *, size_t)>>> EFIBoot::Device_path::deserializers__instance;

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
    BootEntry value;
    value.description = QString::fromStdU16String(load_option.description);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");

    value.optional_data_format = OptionalDataFormat::Base64;
    {
        QTextCodec::ConverterState state;
        value.optional_data = codec->toUnicode(reinterpret_cast<const char *>(load_option.optional_data.data()), static_cast<int>(load_option.optional_data.size()), &state);
        if(state.invalidChars == 0 && !value.optional_data.contains(QChar(0)))
            value.optional_data_format = OptionalDataFormat::Utf8;
    }

    if(value.optional_data_format == OptionalDataFormat::Base64 && load_option.optional_data.size() % sizeof(char16_t) == 0)
    {
        codec = QTextCodec::codecForName("UTF-16");
        QTextCodec::ConverterState state;
        value.optional_data = codec->toUnicode(reinterpret_cast<const char *>(load_option.optional_data.data()), static_cast<int>(load_option.optional_data.size()), &state);
        if(state.invalidChars == 0 && !value.optional_data.contains(QChar(0)))
            value.optional_data_format = OptionalDataFormat::Utf16;
    }

    if(value.optional_data_format == OptionalDataFormat::Base64)
        value.optional_data = QByteArray::fromRawData(reinterpret_cast<const char *>(load_option.optional_data.data()), static_cast<int>(load_option.optional_data.size())).toBase64();

    value.attributes = load_option.attributes;

    for(const auto &device_path: load_option.file_path)
        value.file_path.push_back(std::visit([](const auto &path) -> Device_path::ANY
            { return path; },
            device_path));

    return value;
}

auto BootEntry::toEFIBootLoadOption() const -> EFIBoot::Load_option
{
    EFIBoot::Load_option load_option;
    load_option.description = description.toStdU16String();
    {
        auto bytes = get_raw_optional_data();
        auto begin = reinterpret_cast<const EFIBoot::Raw_data::value_type *>(bytes.constData());
        std::copy(begin, std::next(begin, bytes.size()), std::back_inserter(load_option.optional_data));
    }

    load_option.attributes = attributes;
    for(const auto &device_path: file_path)
        load_option.file_path.push_back(std::visit([](const auto &obj) -> EFIBoot::Device_path::ANY
            { return obj.toEFIBootDevicePath(); },
            device_path));

    return load_option;
}

auto BootEntry::fromJSON(const QJsonObject &obj) -> std::optional<BootEntry>
{
    BootEntry value;
    try_read(description, String);
    try_read_3(optional_data_format, Double, Int);
    try_read(optional_data, String);
    try_read_3(attributes, Double, Int);
    try_read_3(efi_attributes, Double, Int);
    check_type(file_path, Array);
    const auto file_path = obj["file_path"].toArray();
    for(const auto &device_path: file_path)
    {
        auto dp = device_path.toObject();
        auto path = get_default(Device_path::JSON_readers(), QString("%1/%2").arg(dp["type"].toString(), dp["subtype"].toString()), [](const auto &)
            { return std::nullopt; })(dp);
        if(!path)
            return std::nullopt;

        value.file_path.push_back(*path);
    }

    return {value};
}

auto BootEntry::toJSON() const -> QJsonObject
{
    QJsonObject load_option;
    load_option["description"] = description;
    load_option["optional_data_format"] = optional_data_format;
    load_option["optional_data"] = optional_data;
    load_option["attributes"] = static_cast<int>(attributes);
    load_option["efi_attributes"] = static_cast<int>(efi_attributes);
    QJsonArray file_path_json;
    for(const auto &device_path: file_path)
        file_path_json.push_back(std::visit([](const auto &obj) -> QJsonObject
            { return obj.toJSON(); },
            device_path));

    load_option["file_path"] = file_path_json;
    return load_option;
}

auto BootEntry::format_file_path(bool refresh) const -> QString
{
    if(file_path.empty())
        return {};

    if(file_path_str.size() && !refresh)
        return file_path_str;

    file_path_str.clear();
    for(const auto &device_path: file_path)
    {
        if(!file_path_str.isEmpty())
            file_path_str += "/";

        file_path_str += std::visit([refresh](const auto &obj)
            { return obj.toString(refresh); },
            device_path);
    }

    return file_path_str;
}

auto BootEntry::change_optional_data_format(BootEntry::OptionalDataFormat format) -> bool
{
    if(format == optional_data_format)
        return true;

    QTextCodec *codec = nullptr;
    auto bytes = get_raw_optional_data();
    QTextCodec::ConverterState state;
    QString temp_optional_data;
    switch(static_cast<OptionalDataFormat>(format))
    {
    case OptionalDataFormat::Base64:
        temp_optional_data = bytes.toBase64();
        break;

    case OptionalDataFormat::Utf16:
        if(static_cast<uint>(bytes.size()) % sizeof(char16_t) != 0)
            return false;

        codec = QTextCodec::codecForName("UTF-16");
        temp_optional_data = codec->toUnicode(bytes.constData(), static_cast<int>(bytes.size()), &state);
        if(state.invalidChars != 0)
            return false;

        break;

    case OptionalDataFormat::Utf8:
        codec = QTextCodec::codecForName("UTF-8");
        temp_optional_data = codec->toUnicode(bytes.constData(), static_cast<int>(bytes.size()), &state);
        if(state.invalidChars != 0)
            return false;

        break;

    case OptionalDataFormat::Hex:
        temp_optional_data = bytes.toHex();
        break;
    }

    if(temp_optional_data.contains(QChar(0)))
        return false;

    optional_data_format = format;
    optional_data = temp_optional_data;
    return true;
}

auto BootEntry::get_raw_optional_data() const -> QByteArray
{
    QByteArray bytes;
    std::unique_ptr<QTextEncoder> encoder = nullptr;
    switch(static_cast<OptionalDataFormat>(optional_data_format))
    {
    case OptionalDataFormat::Base64:
        bytes = QByteArray::fromBase64(optional_data.toUtf8());
        break;

    case OptionalDataFormat::Utf16:
        encoder.reset(QTextCodec::codecForName("UTF-16")->makeEncoder(QTextCodec::IgnoreHeader));
        bytes = encoder->fromUnicode(optional_data);
        break;

    case OptionalDataFormat::Utf8:
        encoder.reset(QTextCodec::codecForName("UTF-8")->makeEncoder(QTextCodec::IgnoreHeader));
        bytes = encoder->fromUnicode(optional_data);
        break;

    case OptionalDataFormat::Hex:
        bytes = QByteArray::fromHex(optional_data.toUtf8());
        break;
    }

    return bytes;
}

Device_path::PCI::PCI(const EFIBoot::Device_path::PCI &pci)
    : function{pci.function}
    , device{pci.device}
{
}

auto Device_path::PCI::toEFIBootDevicePath() const -> EFIBoot::Device_path::PCI
{
    EFIBoot::Device_path::PCI value = {};
    value.function = function;
    value.device = device;
    return value;
}

auto Device_path::PCI::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::PCI>
{
    PCI value;
    check_obj();
    try_read_3(function, Double, Int);
    try_read_3(device, Double, Int);
    return {value};
}

auto Device_path::PCI::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["function"] = static_cast<int>(function);
    value["device"] = static_cast<int>(device);
    return value;
}

auto Device_path::PCI::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Pci(%1, %2)").arg(device).arg(function);
}

Device_path::HID::HID(const EFIBoot::Device_path::HID &_hid)
    : hid{_hid.hid}
    , uid{_hid.uid}
{
}

auto Device_path::HID::toEFIBootDevicePath() const -> EFIBoot::Device_path::HID
{
    EFIBoot::Device_path::HID value = {};
    value.hid = hid;
    value.uid = uid;
    return value;
}

auto Device_path::HID::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::HID>
{
    HID value;
    check_obj();
    try_read_3(hid, Double, Int);
    try_read_3(uid, Double, Int);
    return {value};
}

auto Device_path::HID::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hid"] = static_cast<int>(hid);
    value["uid"] = static_cast<int>(uid);
    return value;
}

auto Device_path::HID::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Acpi(0x%1, 0x%2)").arg(hid, 0, HEX_BASE).arg(uid, 0, HEX_BASE);
}

static_assert(sizeof(Device_path::Vendor::guid) == sizeof(EFIBoot::Device_path::HWVendor::guid));
static_assert(sizeof(Device_path::Vendor::guid) == sizeof(EFIBoot::Device_path::MSGVendor::guid));

Device_path::Vendor::Vendor(const EFIBoot::Device_path::HWVendor &vendor)
    : _type{vendor.TYPE}
    , data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size()))}
{
    static_assert(sizeof(vendor.guid) == sizeof(guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

Device_path::Vendor::Vendor(const EFIBoot::Device_path::MSGVendor &vendor)
    : _type{vendor.TYPE}
    , data{QByteArray::fromRawData(reinterpret_cast<const char *>(vendor.data.data()), static_cast<int>(vendor.data.size()))}
{
    static_assert(sizeof(vendor.guid) == sizeof(guid));
    memcpy(reinterpret_cast<void *>(&guid), &vendor.guid, sizeof(vendor.guid));
}

auto Device_path::Vendor::toEFIBootDevicePath() const -> EFIBoot::Device_path::ANY
{
    switch(_type)
    {
    case EFIBoot::Device_path::HWVendor::TYPE:
    {
        EFIBoot::Device_path::HWVendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }
    break;

    case EFIBoot::Device_path::MSGVendor::TYPE:
    {
        EFIBoot::Device_path::MSGVendor value = {};
        static_assert(sizeof(guid) == sizeof(value.guid));
        memcpy(value.guid.data(), &guid, sizeof(guid));
        value.data.resize(static_cast<size_t>(data.size()));
        std::copy(std::begin(data), std::end(data), std::begin(value.data));
        return value;
    }
    break;
    }

    return {};
}

auto Device_path::Vendor::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::Vendor>
{
    Vendor value;
    check_obj();
    try_read_3(_type, Double, Int);
    check_type(guid, String);
    value.guid = QUuid::fromString(obj["guid"].toString());
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto Device_path::Vendor::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["_type"] = _type;
    value["guid"] = guid.toString();
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto Device_path::Vendor::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    const char *type_string = "Unk";
    switch(_type)
    {
    case EFIBoot::Device_path::HWVendor::TYPE:
        type_string = "Hw";
        break;

    case EFIBoot::Device_path::MSGVendor::TYPE:
        type_string = "Msg";
        break;
    }

    return string = QString("Ven%1(%2, [%3B])").arg(type_string, guid.toString(QUuid::WithoutBraces)).arg(data.size());
}

Device_path::MACAddress::MACAddress(const EFIBoot::Device_path::MAC_address &mac_address)
    : address{QByteArray::fromRawData(reinterpret_cast<const char *>(mac_address.address.data()), static_cast<int>(mac_address.address.size())).toHex()}
    , if_type{mac_address.if_type}
{
}

auto Device_path::MACAddress::toEFIBootDevicePath() const -> EFIBoot::Device_path::MAC_address
{
    EFIBoot::Device_path::MAC_address value = {};
    auto address_bytes = QByteArray::fromHex(address.toUtf8());
    memcpy(value.address.data(), address_bytes.data(), value.address.size());
    value.if_type = if_type;
    return value;
}

auto Device_path::MACAddress::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::MACAddress>
{
    MACAddress value;
    check_obj();
    try_read(address, String);
    try_read_3(if_type, Double, Int);
    return {value};
}

auto Device_path::MACAddress::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["address"] = address;
    value["if_type"] = if_type;
    return value;
}

auto Device_path::MACAddress::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("MAC(%1, %2)").arg(address.left(12)).arg(if_type);
}

static_assert(sizeof(Device_path::IPv4::local_ip_address.toIPv4Address()) == sizeof(EFIBoot::Device_path::IPv4::local_ip_address));
static_assert(sizeof(Device_path::IPv4::remote_ip_address.toIPv4Address()) == sizeof(EFIBoot::Device_path::IPv4::remote_ip_address));
static_assert(sizeof(Device_path::IPv4::gateway_ip_address.toIPv4Address()) == sizeof(EFIBoot::Device_path::IPv4::gateway_ip_address));
static_assert(sizeof(Device_path::IPv4::subnet_mask.toIPv4Address()) == sizeof(EFIBoot::Device_path::IPv4::subnet_mask));

Device_path::IPv4::IPv4(const EFIBoot::Device_path::IPv4 &ipv4)
    : local_ip_address{*reinterpret_cast<const quint32 *>(ipv4.local_ip_address.data())}
    , remote_ip_address{*reinterpret_cast<const quint32 *>(ipv4.remote_ip_address.data())}
    , local_port{ipv4.local_port}
    , remote_port{ipv4.remote_port}
    , protocol{ipv4.protocol}
    , static_ip_address{ipv4.static_ip_address}
    , gateway_ip_address{*reinterpret_cast<const quint32 *>(ipv4.gateway_ip_address.data())}
    , subnet_mask{*reinterpret_cast<const quint32 *>(ipv4.subnet_mask.data())}
{
}

auto Device_path::IPv4::toEFIBootDevicePath() const -> EFIBoot::Device_path::IPv4
{
    EFIBoot::Device_path::IPv4 value = {};
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

auto Device_path::IPv4::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::IPv4>
{
    IPv4 value;
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

auto Device_path::IPv4::toJSON() const -> QJsonObject
{
    QJsonObject value;
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

auto Device_path::IPv4::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("IPv4(%1:%2, %3, %4, %5:%6, %7, %8)").arg(remote_ip_address.toString()).arg(remote_port).arg(protocol).arg(static_ip_address ? "Static" : "DHCP", local_ip_address.toString()).arg(local_port).arg(gateway_ip_address.toString(), subnet_mask.toString());
}

static_assert(sizeof(Device_path::IPv6::local_ip_address.toIPv6Address()) == sizeof(EFIBoot::Device_path::IPv6::local_ip_address));
static_assert(sizeof(Device_path::IPv6::remote_ip_address.toIPv6Address()) == sizeof(EFIBoot::Device_path::IPv6::remote_ip_address));
static_assert(sizeof(Device_path::IPv6::gateway_ip_address.toIPv6Address()) == sizeof(EFIBoot::Device_path::IPv6::gateway_ip_address));

Device_path::IPv6::IPv6(const EFIBoot::Device_path::IPv6 &ipv6)
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

auto Device_path::IPv6::toEFIBootDevicePath() const -> EFIBoot::Device_path::IPv6
{
    EFIBoot::Device_path::IPv6 value = {};
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

auto Device_path::IPv6::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::IPv6>
{
    IPv6 value;
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

auto Device_path::IPv6::toJSON() const -> QJsonObject
{
    QJsonObject value;
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

auto Device_path::IPv6::toString(bool refresh) const -> QString
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

    return string = QString("IPv6(%1:%2, %3, %4, %5:%6, %7, %8)").arg(remote_ip_address.toString()).arg(remote_port).arg(protocol).arg(origin, local_ip_address.toString()).arg(local_port).arg(gateway_ip_address.toString()).arg(prefix_length);
}

Device_path::SATA::SATA(const EFIBoot::Device_path::SATA &sata)
    : hba_port{sata.hba_port}
    , port_multiplier_port{sata.port_multiplier_port}
    , lun{sata.lun}
{
}

auto Device_path::SATA::toEFIBootDevicePath() const -> EFIBoot::Device_path::SATA
{
    EFIBoot::Device_path::SATA value = {};
    value.hba_port = hba_port;
    value.port_multiplier_port = port_multiplier_port;
    value.lun = lun;
    return value;
}

auto Device_path::SATA::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::SATA>
{
    SATA value;
    check_obj();
    try_read_3(hba_port, Double, Int);
    try_read_3(port_multiplier_port, Double, Int);
    try_read_3(lun, Double, Int);
    return {value};
}

auto Device_path::SATA::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = TYPE;
    value["subtype"] = SUBTYPE;
    value["hba_port"] = static_cast<int>(hba_port);
    value["port_multiplier_port"] = static_cast<int>(port_multiplier_port);
    value["lun"] = static_cast<int>(lun);
    return value;
}

auto Device_path::SATA::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Sata(%1, %2, %3)").arg(hba_port).arg(port_multiplier_port).arg(lun);
}

static_assert(sizeof(Device_path::HD::partition_signature) == sizeof(EFIBoot::Device_path::HD::partition_signature));

Device_path::HD::HD(const EFIBoot::Device_path::HD &hd)
    : partition_start{hd.partition_start}
    , partition_size{hd.partition_size}
    , partition_number{hd.partition_number}
    , partition_format{hd.partition_format}
    , signature_type{hd.signature_type}
{
    static_assert(sizeof(hd.partition_signature) == sizeof(partition_signature));
    memcpy(reinterpret_cast<void *>(&partition_signature), &hd.partition_signature, sizeof(hd.partition_signature));
}

auto Device_path::HD::toEFIBootDevicePath() const -> EFIBoot::Device_path::HD
{
    EFIBoot::Device_path::HD hd;
    hd.partition_start = partition_start;
    hd.partition_size = partition_size;
    hd.partition_number = partition_number;
    hd.partition_format = partition_format;
    static_assert(sizeof(partition_signature) == sizeof(hd.partition_signature));
    memcpy(hd.partition_signature.data(), &partition_signature, sizeof(partition_signature));
    hd.signature_type = signature_type;
    return hd;
}

auto Device_path::HD::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::HD>
{
    HD value;
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

auto Device_path::HD::toJSON() const -> QJsonObject
{
    QJsonObject hd;
    hd["type"] = TYPE;
    hd["subtype"] = SUBTYPE;
    hd["partition_start"] = QString("0x%1").arg(partition_start, 0, HEX_BASE);
    hd["partition_size"] = QString("0x%1").arg(partition_size, 0, HEX_BASE);
    hd["partition_number"] = static_cast<int>(partition_number);
    hd["partition_format"] = static_cast<int>(partition_format);
    hd["partition_signature"] = partition_signature.toString();
    hd["signature_type"] = static_cast<int>(signature_type);
    return hd;
}

auto Device_path::HD::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    QString format = QString("HD(%1, %2, %3. 0x%4, 0x%5)").arg(partition_number);
    switch(signature_type)
    {
    case EFIBoot::Device_path::SIGNATURE::MBR:
    {
        format = format.arg("MBR", QString("0x%1").arg(partition_signature.data1, 0, HEX_BASE));
        break;
    }

    case EFIBoot::Device_path::SIGNATURE::GUID:
        format = format.arg("GPT", partition_signature.toString(QUuid::WithoutBraces));
        break;

    default:
        format = format.arg(signature_type).arg("N/A");
        break;
    }

    return string = format.arg(partition_start, 0, HEX_BASE).arg(partition_size, 0, HEX_BASE);
}

Device_path::File::File(const EFIBoot::Device_path::File &file)
    : name{QString::fromStdU16String(file.name)}
{
}

auto Device_path::File::toEFIBootDevicePath() const -> EFIBoot::Device_path::File
{
    EFIBoot::Device_path::File file;
    file.name = name.toStdU16String();
    return file;
}

auto Device_path::File::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::File>
{
    File value;
    check_obj();
    try_read(name, String);
    return {value};
}

auto Device_path::File::toJSON() const -> QJsonObject
{
    QJsonObject file;
    file["type"] = TYPE;
    file["subtype"] = SUBTYPE;
    file["name"] = name;
    return file;
}

auto Device_path::File::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("File(%1)").arg(name);
}

static_assert(sizeof(Device_path::FirmwareFile::name) == sizeof(EFIBoot::Device_path::Firmware_file::name));

Device_path::FirmwareFile::FirmwareFile(const EFIBoot::Device_path::Firmware_file &firmware_file)
{
    static_assert(sizeof(firmware_file.name) == sizeof(name));
    memcpy(reinterpret_cast<void *>(&name), &firmware_file.name, sizeof(firmware_file.name));
}

auto Device_path::FirmwareFile::toEFIBootDevicePath() const -> EFIBoot::Device_path::Firmware_file
{
    EFIBoot::Device_path::Firmware_file firmware_file;
    static_assert(sizeof(name) == sizeof(firmware_file.name));
    memcpy(firmware_file.name.data(), &name, sizeof(name));
    return firmware_file;
}

auto Device_path::FirmwareFile::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::FirmwareFile>
{
    FirmwareFile value;
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto Device_path::FirmwareFile::toJSON() const -> QJsonObject
{
    QJsonObject firmware_file;
    firmware_file["type"] = TYPE;
    firmware_file["subtype"] = SUBTYPE;
    firmware_file["name"] = name.toString();
    return firmware_file;
}

auto Device_path::FirmwareFile::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("FvFile(%1)").arg(name.toString(QUuid::WithoutBraces));
}

static_assert(sizeof(Device_path::FirmwareVolume::name) == sizeof(EFIBoot::Device_path::Firmware_volume::name));

Device_path::FirmwareVolume::FirmwareVolume(const EFIBoot::Device_path::Firmware_volume &firmware_volume)
{
    static_assert(sizeof(firmware_volume.name) == sizeof(name));
    memcpy(reinterpret_cast<void *>(&name), &firmware_volume.name, sizeof(firmware_volume.name));
}

auto Device_path::FirmwareVolume::toEFIBootDevicePath() const -> EFIBoot::Device_path::Firmware_volume
{
    EFIBoot::Device_path::Firmware_volume firmware_volume;
    static_assert(sizeof(name) == sizeof(firmware_volume.name));
    memcpy(firmware_volume.name.data(), &name, sizeof(name));
    return firmware_volume;
}

auto Device_path::FirmwareVolume::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::FirmwareVolume>
{
    FirmwareVolume value;
    check_obj();
    check_type(name, String);
    value.name = QUuid::fromString(obj["name"].toString());
    return {value};
}

auto Device_path::FirmwareVolume::toJSON() const -> QJsonObject
{
    QJsonObject firmware_volume;
    firmware_volume["type"] = TYPE;
    firmware_volume["subtype"] = SUBTYPE;
    firmware_volume["name"] = name.toString();
    return firmware_volume;
}

auto Device_path::FirmwareVolume::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Fv(%1)").arg(name.toString(QUuid::WithoutBraces));
}

auto Device_path::End::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::End>
{
    End value;
    check_obj();
    try_read_3(_subtype, Double, Int);
    return {value};
}

auto Device_path::End::toJSON() const -> QJsonObject
{
    QJsonObject end_instance;
    end_instance["type"] = TYPE;
    end_instance["subtype"] = SUBTYPE;
    end_instance["_subtype"] = _subtype;
    return end_instance;
}

auto Device_path::End::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    const char *subtype_string = "Unknown";
    switch(_subtype)
    {
    case EFIBoot::EFIDP_END_INSTANCE:
        subtype_string = "Instance";
        break;

    case EFIBoot::EFIDP_END_ENTIRE:
        subtype_string = "Entire";
        break;
    }

    return string = QString("End(%1)").arg(subtype_string);
}

Device_path::Unknown::Unknown(const EFIBoot::Device_path::Unknown &unknown)
    : type{unknown.TYPE}
    , subtype{unknown.SUBTYPE}
    , data{QByteArray::fromRawData(reinterpret_cast<const char *>(unknown.data.data()), static_cast<int>(unknown.data.size()))}
{
}

auto Device_path::Unknown::toEFIBootDevicePath() const -> EFIBoot::Device_path::Unknown
{
    EFIBoot::Device_path::Unknown value = {};
    value.TYPE = type;
    value.SUBTYPE = subtype;
    value.data.resize(static_cast<size_t>(data.size()));
    std::copy(std::begin(data), std::end(data), std::begin(value.data));
    return value;
}

auto Device_path::Unknown::fromJSON(const QJsonObject &obj) -> std::optional<Device_path::Unknown>
{
    Unknown value;
    check_obj();
    try_read_3(type, Double, Int);
    try_read_3(subtype, Double, Int);
    check_type(data, String);
    value.data = QByteArray::fromBase64(obj["data"].toString().toUtf8());
    return {value};
}

auto Device_path::Unknown::toJSON() const -> QJsonObject
{
    QJsonObject value;
    value["type"] = type;
    value["subtype"] = subtype;
    value["data"] = static_cast<QString>(data.toBase64());
    return value;
}

auto Device_path::Unknown::toString(bool refresh) const -> QString
{
    if(string.size() && !refresh)
        return string;

    return string = QString("Unknown(0x%1, 0x%2, [%3B])").arg(type, 0, HEX_BASE).arg(subtype, 0, HEX_BASE).arg(data.size());
}

#undef try_read_3
#undef try_read
#undef check_type
#undef check_obj
