// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "hotkey.h"

#include <QJsonArray>
#include <QJsonObject>

#include "efiboot.h"

#define check_type(field, typecheck)                          \
    if(!obj.contains(#field) || !obj[#field].is##typecheck()) \
    return std::nullopt
#define try_read_3(field, typecheck, typecast) \
    check_type(field, typecheck);              \
    value.field = static_cast<decltype(value.field)>(obj[#field].to##typecast())

auto HotKey::fromEFIBootKeyOption(
    const EFIBoot::Key_option &key_option) -> HotKey
{
    HotKey value{};
    value.boot_option = key_option.boot_option;
    value.keys = {key_option.key_data, key_option.keys};
    value.vendor_data = QByteArray::fromRawData(reinterpret_cast<const char *>(key_option.vendor_data.data()), static_cast<int>(key_option.vendor_data.size()));
    value.vendor_data.detach();
    return value;
}

auto HotKey::fromError(const QString &error) -> HotKey
{
    HotKey value{};
    value.is_error = true;
    value.error = error;
    return value;
}

auto HotKey::toEFIBootKeyOption(const std::unordered_map<uint16_t, uint32_t> &crc32) const -> EFIBoot::Key_option
{
    if(is_error)
        return {};

    EFIBoot::Key_option key_option{};
    key_option.boot_option = boot_option;
    if(const auto crc = crc32.find(boot_option); crc != crc32.end())
        key_option.boot_option_crc = crc->second;

    if(!keys.toEFIKeyOption(key_option.key_data, key_option.keys))
        return {};

    auto begin = reinterpret_cast<const EFIBoot::Raw_data::value_type *>(vendor_data.constData());
    std::copy(begin, std::next(begin, vendor_data.size()), std::back_inserter(key_option.vendor_data));
    return key_option;
}

auto HotKey::fromJSON(const QJsonObject &obj, qsizetype maxKeys) -> std::optional<HotKey>
{
    HotKey value{};
    try_read_3(boot_option, Double, Int);
    check_type(keys, String);
    const auto keys = obj["keys"].toString();
    value.keys = EFIKeySequence::fromString(keys, maxKeys);
    if(value.keys.isEmpty())
        return {};

    check_type(vendor_data, String);
    value.vendor_data = QByteArray::fromBase64(obj["vendor_data"].toString().toUtf8());
    return {value};
}

auto HotKey::toJSON() const -> QJsonObject
{
    if(is_error)
        return {};

    QJsonObject key_option;
    key_option["boot_option"] = boot_option;
    key_option["keys"] = keys.toString();
    key_option["vendor_data"] = QString{vendor_data.toBase64()};
    return key_option;
}

#undef try_read_3
#undef check_type
