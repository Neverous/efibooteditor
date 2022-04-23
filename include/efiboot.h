// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <array>
#include <charconv>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "compat.h"

template <class... Ts>
struct overloaded: Ts...
{
    using Ts::operator()...;
    overloaded(const overloaded &) = delete;
    overloaded &operator=(const overloaded &) = delete;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace EFIBoot
{

extern "C"
{
#include "efivar-lite/efiboot-loadopt.h"
#include "efivar-lite/efivar.h"
}

inline bool operator==(const efi_guid_t &first, const efi_guid_t &second)
{
    return efi_guid_cmp(&first, &second) == 0;
}

typedef std::vector<uint8_t> Raw_data;

template <class Type = Raw_data>
std::optional<Type> deserialize(const void *data, size_t data_size);

namespace Device_path
{

template <class Type>
inline bool register_deserializer();
#define REGISTER_DESERIALIZER(type) static const bool is_##type##_deserializer_registered = register_deserializer<type>()

#if defined(_MSC_VER)
#pragma warning(push)
// C4820: 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable : 4820)
#endif

enum TYPE
{
    HW = 0x01,
    ACPI = 0x02,
    MSG = 0x03,
    MEDIA = 0x04,
    END = 0x7F,
};

struct PCI
{
    static const uint8_t TYPE = HW;
    static const uint8_t SUBTYPE = 0x01;

    uint8_t function = 0;
    uint8_t device = 0;
};
REGISTER_DESERIALIZER(PCI);

struct HID
{
    static const uint8_t TYPE = ACPI;
    static const uint8_t SUBTYPE = 0x01;

    uint32_t hid = 0;
    uint32_t uid = 0;
};
REGISTER_DESERIALIZER(HID);

struct Vendor
{
    static const uint8_t TYPE = MSG;
    static const uint8_t SUBTYPE = 0x0a;

    std::array<uint8_t, 16> guid = {};
    Raw_data data = {};
};
REGISTER_DESERIALIZER(Vendor);

struct MAC_address
{
    static const uint8_t TYPE = MSG;
    static const uint8_t SUBTYPE = 0x0b;

    std::array<uint8_t, 32> address = {};
    uint8_t if_type = 0;
};
REGISTER_DESERIALIZER(MAC_address);

struct IPv4
{
    static const uint8_t TYPE = MSG;
    static const uint8_t SUBTYPE = 0x0c;

    std::array<uint8_t, 4> local_ip_address = {};
    std::array<uint8_t, 4> remote_ip_address = {};
    uint16_t local_port = 0;
    uint16_t remote_port = 0;
    uint16_t protocol = 0;
    bool static_ip_address = false;
    std::array<uint8_t, 4> gateway_ip_address = {};
    std::array<uint8_t, 4> subnet_mask = {};
};
REGISTER_DESERIALIZER(IPv4);

struct IPv6
{
    static const uint8_t TYPE = MSG;
    static const uint8_t SUBTYPE = 0x0d;

    std::array<uint8_t, 16> local_ip_address = {};
    std::array<uint8_t, 16> remote_ip_address = {};
    uint16_t local_port = 0;
    uint16_t remote_port = 0;
    uint16_t protocol = 0;
    uint8_t ip_address_origin = 0;
    uint8_t prefix_length = 0;
    std::array<uint8_t, 16> gateway_ip_address = {};
};
REGISTER_DESERIALIZER(IPv6);

struct SATA
{
    static const uint8_t TYPE = MSG;
    static const uint8_t SUBTYPE = 0x12;

    uint16_t hba_port = 0;
    uint16_t port_multiplier_port = 0;
    uint16_t lun = 0;
};
REGISTER_DESERIALIZER(SATA);

enum SIGNATURE
{
    NONE = 0x00,
    MBR = 0x01,
    GUID = 0x02,
};

struct HD
{
    static const uint8_t TYPE = MEDIA;
    static const uint8_t SUBTYPE = 0x01;

    uint64_t partition_start = 0;
    uint64_t partition_size = 0;
    uint32_t partition_number = 0;
    uint8_t partition_format = 0;
    std::array<uint8_t, 16> partition_signature = {};
    uint8_t signature_type = 0;
};
REGISTER_DESERIALIZER(HD);

struct File
{
    static const uint8_t TYPE = MEDIA;
    static const uint8_t SUBTYPE = 0x04;

    std::u16string name = u"";
};
REGISTER_DESERIALIZER(File);

struct Firmware_file
{
    static const uint8_t TYPE = MEDIA;
    static const uint8_t SUBTYPE = 0x06;

    std::array<uint8_t, 16> name = {};
};
REGISTER_DESERIALIZER(Firmware_file);

struct Firmware_volume
{
    static const uint8_t TYPE = MEDIA;
    static const uint8_t SUBTYPE = 0x07;

    std::array<uint8_t, 16> name = {};
};
REGISTER_DESERIALIZER(Firmware_volume);

struct End
{
    static const uint8_t TYPE = END;
    static const uint8_t SUBTYPE = 0xFF;
};

typedef std::variant<
    PCI,
    HID,
    Vendor,
    MAC_address,
    IPv4,
    IPv6,
    SATA,
    HD,
    File,
    Firmware_file,
    Firmware_volume>
    ANY;

extern std::unique_ptr<std::unordered_map<uint16_t, std::function<std::optional<ANY>(const void *, size_t)>>> deserializers__instance;

inline auto &deserializers()
{
    if(!deserializers__instance)
        deserializers__instance = std::make_unique<decltype(deserializers__instance)::element_type>();

    return *deserializers__instance;
}

template <class Type>
inline bool register_deserializer()
{
    efidp_data dp;
    dp.header.type = Type::TYPE;
    dp.header.subtype = Type::SUBTYPE;

    if(deserializers().find(dp._type_subtype) != deserializers().end())
        return true;

    deserializers()[dp._type_subtype] = [](const void *data, size_t data_size) -> std::optional<ANY>
    { return deserialize<Type>(data, data_size); };
    return true;
}

#undef REGISTER_DESERIALIZER
} // namespace Device_path

struct Load_option
{
    std::u16string description = u"";
    std::vector<Device_path::ANY> file_path = {};
    Raw_data optional_data = {};
    uint32_t attributes = 0;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

enum Load_option_attribute : uint32_t
{
    LOAD_OPTION_EMPTY = 0x00000000,

    LOAD_OPTION_ACTIVE = 0x00000001,
    LOAD_OPTION_FORCE_RECONNECT = 0x00000002,
    LOAD_OPTION_HIDDEN = 0x00000008,

    LOAD_OPTION_CATEGORY_MASK = 0x00001F00,
    LOAD_OPTION_CATEGORY_BOOT = 0x00000000,
    LOAD_OPTION_CATEGORY_APP = 0x00000100,
};

typedef std::function<bool(const efi_guid_t &, const std::tstring_view)> Filter_fn;
typedef std::function<const void *(const void *, const size_t)> Advance_fn;
typedef std::function<size_t(const void *)> Size_fn;

template <class Type = Raw_data>
using Variable = std::tuple<Type, uint32_t>;

std::optional<std::tstring> init();

template <class Type = Raw_data>
std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size);

template <class Type = Raw_data>
std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size, Size_fn get_element_size, Advance_fn get_next_element);

template <class Type = Raw_data>
size_t serialize(Raw_data &output, const Type &value);

template <class Type = Raw_data>
size_t serialize_list(Raw_data &output, const std::vector<Type> &value);

std::unordered_map<std::tstring, efi_guid_t> get_variables(Filter_fn filter);
std::unordered_map<std::tstring, efi_guid_t> get_variables();

template <class Type = Raw_data>
std::optional<Variable<Type>> get_variable(const efi_guid_t &guid, const std::tstring &name);

template <class Type = Raw_data>
std::optional<Variable<std::vector<Type>>> get_list_variable(const efi_guid_t &guid, const std::tstring &name);

template <class Type = Raw_data>
bool set_variable(const efi_guid_t &guid, const std::tstring &name, const Variable<Type> &variable, mode_t mode);

template <class Type = Raw_data>
bool set_list_variable(const efi_guid_t &guid, const std::tstring &name, const Variable<std::vector<Type>> &variable, mode_t mode);

inline std::optional<std::tstring> init()
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
    const uint8_t *ptr = static_cast<const uint8_t *>(data);
    return {Raw_data{ptr, ptr + data_size}};
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
    size_t bytes = value.size() * sizeof(std::string::value_type);
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
    size_t bytes = value.size() * sizeof(std::wstring::value_type);
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
    size_t bytes = value.size() * sizeof(std::u16string::value_type);
    size_t pos = output.size();
    output.resize(pos + bytes);
    memcpy(&output[pos], value.c_str(), bytes);
    return bytes;
}

template <class Type>
inline std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size, Size_fn get_element_size, Advance_fn get_next_element)
{
    std::vector<Type> values;
    const void *data_end = static_cast<const void *>(static_cast<const uint8_t *>(data) + data_size);
    while(data && data < data_end)
    {
        auto element_size = get_element_size(data);
        auto value = deserialize<Type>(data, element_size);
        if(!value)
            return std::nullopt;

        values.push_back(*value);
        data = get_next_element(data, data_size);
        auto bytes_left = static_cast<const uint8_t *>(data_end) - static_cast<const uint8_t *>(data);
        data_size = reinterpret_cast<size_t &>(bytes_left);
    }

    if(data != data_end)
        return std::nullopt;

    return {values};
}

template <class Type>
inline std::optional<std::vector<Type>> deserialize_list(const void *data, size_t data_size)
{
    return deserialize_list<Type>(
        data, data_size,
        [](const void *) -> size_t
        {
            return sizeof(Type);
        },
        [](const void *ptr, size_t) -> const void *
        {
            return static_cast<const void *>(static_cast<const Type *>(ptr) + 1);
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

template <>
inline std::optional<Load_option> deserialize(const void *data, size_t data_size)
{
    Load_option value;
    ssize_t ssize = reinterpret_cast<ssize_t &>(data_size);
    efi_load_option *load_option = const_cast<efi_load_option *>(static_cast<const efi_load_option *>(data));

    for(size_t d = 0; load_option->description[d]; ++d)
        value.description.push_back(load_option->description[d]);

    uint16_t device_path_size = efi_loadopt_pathlen(load_option, ssize);
    efidp device_path = efi_loadopt_path(load_option, ssize);

    auto file_path = deserialize_list<Device_path::ANY>(
        device_path, device_path_size,
        [](const void *ptr) -> size_t
        {
            auto size = efidp_node_size(static_cast<const_efidp>(ptr));
            return reinterpret_cast<size_t &>(size);
        },
        [](const void *ptr, const size_t bytes_left) -> const void *
        {
            const_efidp dp = static_cast<const_efidp>(ptr);
            ssize_t size = efidp_node_size(dp);
            if(reinterpret_cast<size_t &>(size) > bytes_left)
                return nullptr;

            if(reinterpret_cast<size_t &>(size) == bytes_left)
                return static_cast<const uint8_t *>(ptr) + bytes_left;

            const_efidp next = nullptr;
            if(efidp_next_instance(dp, &next) < 0 && efidp_next_node(dp, &next) < 0)
                return nullptr;

            return next;
        });

    if(!file_path || file_path->empty())
        return std::nullopt;

    file_path->pop_back(); // pop END tag
    value.file_path = *file_path;

    uint8_t *optional_data = nullptr;
    size_t optional_data_size = 0;
    int ret = efi_loadopt_optional_data(load_option, data_size, &optional_data, &optional_data_size);
    if(ret >= 0)
    {
        auto opt_data = deserialize<Raw_data>(optional_data, optional_data_size);
        if(!opt_data)
            return std::nullopt;

        value.optional_data = *opt_data;
    }

    value.attributes = load_option->attributes;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::End &)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::END;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::End::SUBTYPE;
    bytes += serialize(output, subtype);
    uint16_t length = static_cast<uint16_t>(bytes + sizeof(uint16_t));
    bytes += serialize(output, length);
    return bytes;
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
    std::u16string::value_type zero = 0;
    size += serialize(output, zero);
    {
        file_path_list_size = static_cast<uint16_t>(serialize_list(output, load_option.file_path));
        Device_path::End end;
        file_path_list_size += static_cast<uint16_t>(serialize(output, end));
        size += file_path_list_size;
        memcpy(&output[file_path_list_length_pos], &file_path_list_size, sizeof(file_path_list_size));
    }

    size += serialize(output, load_option.optional_data);
    return size;
}

template <>
inline std::optional<Device_path::PCI> deserialize(const void *data, size_t data_size)
{
    const efidp_pci *dp = static_cast<const efidp_pci *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::HW)
        return std::nullopt;

    if(dp->header.subtype != Device_path::PCI::SUBTYPE)
        return std::nullopt;

    Device_path::PCI value;
    value.function = dp->function;
    value.device = dp->device;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::PCI &pci)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::HW;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::PCI::SUBTYPE;
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
inline std::optional<Device_path::HID> deserialize(const void *data, size_t data_size)
{
    const efidp_hid *dp = static_cast<const efidp_hid *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::ACPI)
        return std::nullopt;

    if(dp->header.subtype != Device_path::HID::SUBTYPE)
        return std::nullopt;

    Device_path::HID value;
    value.hid = dp->hid;
    value.uid = dp->uid;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::HID &hid)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::ACPI;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::HID::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, hid.hid);
    bytes += serialize(output, hid.uid);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<Device_path::Vendor> deserialize(const void *data, size_t data_size)
{
    const efidp_vendor *dp = static_cast<const efidp_vendor *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MSG)
        return std::nullopt;

    if(dp->header.subtype != Device_path::Vendor::SUBTYPE)
        return std::nullopt;

    Device_path::Vendor value;
    std::copy(std::begin(dp->guid), std::end(dp->guid), std::begin(value.guid));
    size_t data_length = data_size - sizeof(dp->header) - sizeof(dp->guid) / sizeof(dp->guid[0]);
    value.data.resize(data_length);
    std::copy(std::begin(dp->data), std::next(std::begin(dp->data), static_cast<int>(data_length)), std::begin(value.data));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::Vendor &vendor)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MSG;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::Vendor::SUBTYPE;
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
inline std::optional<Device_path::MAC_address> deserialize(const void *data, size_t data_size)
{
    const efidp_mac_address *dp = static_cast<const efidp_mac_address *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MSG)
        return std::nullopt;

    if(dp->header.subtype != Device_path::MAC_address::SUBTYPE)
        return std::nullopt;

    Device_path::MAC_address value;
    std::copy(std::begin(dp->address), std::end(dp->address), std::begin(value.address));
    value.if_type = dp->if_type;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::MAC_address &mac_address)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MSG;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::MAC_address::SUBTYPE;
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
inline std::optional<Device_path::IPv4> deserialize(const void *data, size_t data_size)
{
    const efidp_ipv4 *dp = static_cast<const efidp_ipv4 *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MSG)
        return std::nullopt;

    if(dp->header.subtype != Device_path::IPv4::SUBTYPE)
        return std::nullopt;

    Device_path::IPv4 value;
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
inline size_t serialize(Raw_data &output, const Device_path::IPv4 &ipv4)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MSG;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::IPv4::SUBTYPE;
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
inline std::optional<Device_path::IPv6> deserialize(const void *data, size_t data_size)
{
    const efidp_ipv6 *dp = static_cast<const efidp_ipv6 *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MSG)
        return std::nullopt;

    if(dp->header.subtype != Device_path::IPv6::SUBTYPE)
        return std::nullopt;

    Device_path::IPv6 value;
    std::copy(std::begin(dp->local_ip_address), std::end(dp->local_ip_address), std::begin(value.local_ip_address));
    std::copy(std::begin(dp->remote_ip_address), std::end(dp->remote_ip_address), std::begin(value.remote_ip_address));
    value.local_port = dp->local_port;
    value.remote_port = dp->remote_port;
    value.protocol = dp->protocol;
    value.ip_address_origin = dp->ip_address_origin;
    value.prefix_length = dp->prefix_length;
    std::copy(std::begin(dp->gateway_ip_address), std::end(dp->gateway_ip_address), std::begin(value.gateway_ip_address));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::IPv6 &ipv6)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MSG;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::IPv6::SUBTYPE;
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
inline std::optional<Device_path::SATA> deserialize(const void *data, size_t data_size)
{
    const efidp_sata *dp = static_cast<const efidp_sata *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MSG)
        return std::nullopt;

    if(dp->header.subtype != Device_path::SATA::SUBTYPE)
        return std::nullopt;

    Device_path::SATA value;
    value.hba_port = dp->hba_port;
    value.port_multiplier_port = dp->port_multiplier_port;
    value.lun = dp->lun;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::SATA &sata)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MSG;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::SATA::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, sata.hba_port);
    bytes += serialize(output, sata.port_multiplier_port);
    bytes += serialize(output, sata.lun);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<Device_path::HD> deserialize(const void *data, size_t data_size)
{
    const efidp_hd *dp = static_cast<const efidp_hd *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MEDIA)
        return std::nullopt;

    if(dp->header.subtype != Device_path::HD::SUBTYPE)
        return std::nullopt;

    Device_path::HD value;
    value.partition_size = dp->partition_size;
    value.partition_start = dp->partition_start;
    value.partition_format = dp->partition_format;
    std::copy(std::begin(dp->partition_signature), std::end(dp->partition_signature), std::begin(value.partition_signature));
    value.signature_type = dp->signature_type;
    value.partition_number = dp->partition_number;
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::HD &hd)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MEDIA;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::HD::SUBTYPE;
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
inline std::optional<Device_path::File> deserialize(const void *data, size_t data_size)
{
    const efidp_file *dp = static_cast<const efidp_file *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MEDIA)
        return std::nullopt;

    if(dp->header.subtype != Device_path::File::SUBTYPE)
        return std::nullopt;

    Device_path::File value;
    const void *name = dp->name;
    value.name = static_cast<const std::u16string::value_type *>(name);
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::File &file)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MEDIA;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::File::SUBTYPE;
    bytes += serialize(output, subtype);
    size_t pos = output.size();
    uint16_t length = 0;
    bytes += serialize(output, length);
    bytes += serialize(output, file.name);
    std::u16string::value_type zero = 0;
    bytes += serialize(output, zero);
    length = static_cast<uint16_t>(bytes);
    memcpy(&output[pos], &length, sizeof(length));
    return bytes;
}

template <>
inline std::optional<Device_path::Firmware_file> deserialize(const void *data, size_t data_size)
{
    const efidp_firmware_file *dp = static_cast<const efidp_firmware_file *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MEDIA)
        return std::nullopt;

    if(dp->header.subtype != Device_path::Firmware_file::SUBTYPE)
        return std::nullopt;

    Device_path::Firmware_file value;
    std::copy(std::begin(dp->name), std::end(dp->name), std::begin(value.name));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::Firmware_file &firmware_file)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MEDIA;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::Firmware_file::SUBTYPE;
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
inline std::optional<Device_path::Firmware_volume> deserialize(const void *data, size_t data_size)
{
    const efidp_firmware_volume *dp = static_cast<const efidp_firmware_volume *>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type != Device_path::TYPE::MEDIA)
        return std::nullopt;

    if(dp->header.subtype != Device_path::Firmware_volume::SUBTYPE)
        return std::nullopt;

    Device_path::Firmware_volume value;
    std::copy(std::begin(dp->name), std::end(dp->name), std::begin(value.name));
    return {value};
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::Firmware_volume &firmware_volume)
{
    size_t bytes = 0;
    uint8_t type = Device_path::TYPE::MEDIA;
    bytes += serialize(output, type);
    uint8_t subtype = Device_path::Firmware_volume::SUBTYPE;
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
inline std::optional<Device_path::ANY> deserialize(const void *data, size_t data_size)
{
    const_efidp dp = static_cast<const_efidp>(data);
    if(dp->header.length != data_size)
        return std::nullopt;

    if(dp->header.type == Device_path::End::TYPE && dp->header.subtype == Device_path::End::SUBTYPE)
        return Device_path::ANY{};

    return get_default(Device_path::deserializers(), dp->_type_subtype, [](const void *, size_t)
        { return std::nullopt; })(dp, data_size);
}

template <>
inline size_t serialize(Raw_data &output, const Device_path::ANY &device_path)
{
    return std::visit([&output](const auto &dp) -> size_t
        { return serialize(output, dp); },
        device_path);
}

inline std::unordered_map<std::tstring, efi_guid_t> get_variables(Filter_fn filter_fn)
{
    std::unordered_map<std::tstring, efi_guid_t> variables;
    efi_guid_t *guid = nullptr;
    TCHAR *name = nullptr;
    while(efi_get_next_variable_name(&guid, &name) > 0)
    {
        if(!filter_fn(*guid, name))
            continue;

        variables[name] = *guid;
    }

    return variables;
}

inline std::unordered_map<std::tstring, efi_guid_t> get_variables()
{
    return get_variables([](const efi_guid_t &, const std::tstring_view)
        { return true; });
}

template <class Type>
inline std::optional<Variable<Type>> get_variable(const efi_guid_t &guid, const std::tstring &name)
{
    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    int ret = efi_get_variable(guid, name.c_str(), &data, &data_size, &attributes);
    if(ret < 0)
        return std::nullopt;

    auto value = deserialize<Type>(data, data_size);
    if(!value)
        return std::nullopt;

    return {{*value, attributes}};
}

template <class Type>
inline std::optional<Variable<std::vector<Type>>> get_list_variable(const efi_guid_t &guid, const std::tstring &name)
{
    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    int ret = efi_get_variable(guid, name.c_str(), &data, &data_size, &attributes);
    if(ret < 0)
        return std::nullopt;

    auto value = deserialize_list<Type>(data, data_size);
    if(!value)
        return std::nullopt;

    return {{*value, attributes}};
}

template <class Type>
inline bool set_variable(const efi_guid_t &guid, const std::tstring &name, const Variable<Type> &variable, mode_t mode)
{
    auto [value, attributes] = variable;
    Raw_data bytes;
    size_t size = serialize(bytes, value);
    return efi_set_variable(guid, name.c_str(), bytes.data(), size, attributes, mode) == 0;
}

template <class Type>
inline bool set_list_variable(const efi_guid_t &guid, const std::tstring &name, const Variable<std::vector<Type>> &variable, mode_t mode)
{
    auto [value, attributes] = variable;
    Raw_data bytes;
    size_t size = serialize_list(bytes, value);
    return efi_set_variable(guid, name.c_str(), bytes.data(), size, attributes, mode) == 0;
}

inline bool del_variable(const efi_guid_t &guid, const std::tstring &name)
{
    return efi_del_variable(guid, name.c_str()) == 0;
}

inline std::tstring get_error_trace()
{
    std::tstring output = _T("Error trace:\n");
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
        output += std::to_tstring(line);
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
