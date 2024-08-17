// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "efikeysequence.h"

EFIKey::EFIKey(const EFIBoot::efi_input_key &key)
{
    if(key.scan_code)
    {
        auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
            { return std::get<uint16_t>(row) == key.scan_code; });

        if(code != efi_scan_codes.end())
            scan_code = std::get<Qt::Key>(*code);
    }
    else
        scan_code = Qt::Key_unknown;

    unicode_char = key.unicode_char;
}

EFIBoot::efi_input_key EFIKey::toEFIInputKey() const
{
    EFIBoot::efi_input_key value;
    if(scan_code != Qt::Key_unknown)
    {
        auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
            { return std::get<Qt::Key>(row) == scan_code; });

        if(code != efi_scan_codes.end())
            value.scan_code = std::get<uint16_t>(*code);
    }
    else
        value.scan_code = 0;

    value.unicode_char = unicode_char.unicode();
    return value;
}

bool EFIKey::operator==(const EFIKey &b) const
{
    return scan_code == b.scan_code && unicode_char == b.unicode_char;
}

EFIKey EFIKey::fromString(const QString &repr, bool *success)
{
    if(success)
        *success = false;

    EFIKey value;
    auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
        { return std::get<QString>(row) == repr; });

    if(code != efi_scan_codes.end())
    {
        value.scan_code = std::get<Qt::Key>(*code);
        if(success)
            *success = true;

        return value;
    }

    value.scan_code = Qt::Key_unknown;
    if(repr.length() != 1)
        return {};

    value.unicode_char = repr[0];
    if(success)
        *success = true;

    return value;
}

QString EFIKey::toString() const
{
    if(scan_code != Qt::Key_unknown)
    {
        auto repr = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
            { return std::get<Qt::Key>(row) == scan_code; });

        if(repr != efi_scan_codes.end())
            return std::get<QString>(*repr);

        return {};
    }

    return unicode_char;
}

EFIKey EFIKey::fromQKey(int keycode, const QString &text, bool *success)
{
    if(success)
        *success = false;

    EFIKey value;
    if(keycode != Qt::Key_unknown)
    {
        auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
            { return std::get<Qt::Key>(row) == keycode; });

        if(code != efi_scan_codes.end())
        {
            value.scan_code = std::get<Qt::Key>(*code);
            if(success)
                *success = true;

            return value;
        }
    }

    value.scan_code = Qt::Key_unknown;
    if(keycode && keycode < Qt::Key_Escape && keycode != Qt::Key_Space)
    {
        value.unicode_char = static_cast<char16_t>(keycode);
        if(success)
            *success = true;

        return value;
    }

    if(text.trimmed().length() != 1)
        return {};

    value.unicode_char = text.trimmed()[0];
    if(success)
        *success = true;

    return value;
}

EFIKeySequence::EFIKeySequence(const EFIBoot::efi_boot_key_data &key_data, const std::vector<EFIBoot::efi_input_key> &keys_)
{
    if(key_data.options.shift_pressed)
        shift_state.insert(Qt::Key_Shift);

    if(key_data.options.control_pressed)
        shift_state.insert(Qt::Key_Control);

    if(key_data.options.alt_pressed)
        shift_state.insert(Qt::Key_Alt);

    if(key_data.options.logo_pressed)
        shift_state.insert(Qt::Key_Meta);

    if(key_data.options.menu_pressed)
        shift_state.insert(Qt::Key_Menu);

    if(key_data.options.sys_req_pressed)
        shift_state.insert(Qt::Key_SysReq);

    for(size_t k = 0; k < key_data.options.input_key_count; ++k)
        keys.push_back(EFIKey(keys_[k]));
}

bool EFIKeySequence::toEFIKeyOption(EFIBoot::efi_boot_key_data &key_data, std::vector<EFIBoot::efi_input_key> &keys_) const
{
    if(keys.size() > 3)
        return false;

    key_data.options.shift_pressed = shift_state.contains(Qt::Key_Shift);
    key_data.options.control_pressed = shift_state.contains(Qt::Key_Control);
    key_data.options.alt_pressed = shift_state.contains(Qt::Key_Alt);
    key_data.options.logo_pressed = shift_state.contains(Qt::Key_Meta);
    key_data.options.menu_pressed = shift_state.contains(Qt::Key_Menu);
    key_data.options.sys_req_pressed = shift_state.contains(Qt::Key_SysReq);

    key_data.options.input_key_count = static_cast<uint32_t>(keys.size());
    for(const auto &key: keys)
        keys_.push_back(key.toEFIInputKey());

    return true;
}

EFIKeySequence EFIKeySequence::fromString(const QString &str, qsizetype maxKeys)
{
    EFIKeySequence value = {};
    const auto keys = str.split("+");
    int k = 0;
    while(k < keys.size())
    {
        auto modif = std::find_if(efi_modifiers.begin(), efi_modifiers.end(), [&](const auto &row)
            { return std::get<QString>(row) == keys[k]; });

        if(modif == efi_modifiers.end())
            break;

        value.shift_state.insert(std::get<Qt::Key>(*modif));
        ++k;
    }

    if(keys.size() - k > maxKeys)
        return {};

    while(k < keys.size())
    {
        if(keys[k].size() > 1)
            return {};

        bool success = false;
        auto key = EFIKey::fromString(keys[k], &success);
        if(!success)
            return {};

        value.keys.push_back(key);
        ++k;
    }

    return value;
}

QString EFIKeySequence::toString() const
{
    QString str;
    for(const auto &[text, keycode]: efi_modifiers)
    {
        if(!shift_state.contains(keycode))
            continue;

        if(!str.isEmpty())
            str += "+";

        str += text;
    }

    for(const auto &key: keys)
    {
        if(!str.isEmpty())
            str += "+";

        str += key.toString();
    }

    return str;
}

bool EFIKeySequence::isEmpty() const
{
    return shift_state.isEmpty() && keys.isEmpty();
}

bool EFIKeySequence::operator==(const EFIKeySequence &b) const
{
    return shift_state == b.shift_state && keys == b.keys;
}

bool EFIKeySequence::addKey(int keycode, const QString &text, qsizetype maxKeys)
{
    if(auto modif = std::find_if(efi_modifiers.begin(), efi_modifiers.end(), [&](const auto &row)
           { return std::get<Qt::Key>(row) == keycode; });
        modif != efi_modifiers.end())
    {
        shift_state.insert(static_cast<Qt::Key>(keycode));
        return true;
    }

    if(keys.size() >= maxKeys)
        return false;

    bool success = false;
    EFIKey key = EFIKey::fromQKey(keycode, text, &success);
    if(!success)
        return false;

    if(!keys.isEmpty() && keys.last() == key)
        return false;

    keys.push_back(key);
    return true;
}
