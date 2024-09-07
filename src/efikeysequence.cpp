// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"

#include "efikeysequence.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>

EFIKey::EFIKey(const EFIBoot::efi_input_key &key)
{
    if(key.scan_code)
    {
        if(auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
               { return std::get<int>(row) == key.scan_code; });
            code != efi_scan_codes.end())
            scan_code = std::get<Qt::Key>(*code);
    }
    else
        scan_code = Qt::Key_unknown;

    unicode_char = key.unicode_char;
}

EFIKey::EFIKey(const Qt::Key _scan_code, const QChar _unicode_char)
    : scan_code{_scan_code}
    , unicode_char{_unicode_char}
{
}

EFIBoot::efi_input_key EFIKey::toEFIInputKey() const
{
    EFIBoot::efi_input_key value;
    if(scan_code != Qt::Key_unknown)
    {
        if(auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
               { return std::get<Qt::Key>(row) == scan_code; });
            code != efi_scan_codes.end())
            value.scan_code = static_cast<uint16_t>(std::get<int>(*code));
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
    if(auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
           { return std::get<QString>(row) == repr; });
        code != efi_scan_codes.end())
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
        if(auto repr = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
               { return std::get<Qt::Key>(row) == scan_code; });
            repr != efi_scan_codes.end())
            return std::get<QString>(*repr);

        return {};
    }

    return unicode_char;
}

EFIKey EFIKey::toUpper() const
{
    return EFIKey{scan_code, unicode_char.toUpper()};
}

bool EFIKey::isUpper() const
{
    return unicode_char.toLower() != unicode_char;
}

EFIKey EFIKey::fromQKey(int key, Qt::KeyboardModifiers modifiers, const QString &text, bool *success)
{
    if(success)
        *success = false;

    EFIKey value;
    if(key != Qt::Key_unknown)
    {
        if(auto code = std::find_if(efi_scan_codes.begin(), efi_scan_codes.end(), [&](const auto &row)
               { return std::get<Qt::Key>(row) == key; });
            code != efi_scan_codes.end())
        {
            value.scan_code = std::get<Qt::Key>(*code);
            if(success)
                *success = true;

            return value;
        }
    }

    value.scan_code = Qt::Key_unknown;
    if(modifiers)
    {
        auto stripped = QKeySequence{key}.toString();
        if(stripped.length() != 1)
            return {};

        if(!(modifiers & Qt::ShiftModifier) && Qt::Key_A <= key && key <= Qt::Key_Z)
            stripped = stripped.toLower();

        value.unicode_char = stripped[0];
        if(success)
            *success = true;

        return value;
    }

    if(text.length() != 1)
        return {};

    value.unicode_char = text[0];
    if(success)
        *success = true;

    return value;
}

EFIKeySequence::EFIKeySequence(const EFIBoot::efi_boot_key_data &key_data, const std::vector<EFIBoot::efi_input_key> &keys_)
{
    bool has_upper = false;
    for(size_t k = 0; k < key_data.options.input_key_count; ++k)
    {
        EFIKey key{keys_[k]};
        keys.push_back(key);
        has_upper |= key.isUpper();
    }

    if(key_data.options.shift_pressed || has_upper)
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
}

bool EFIKeySequence::toEFIKeyOption(EFIBoot::efi_boot_key_data &key_data, std::vector<EFIBoot::efi_input_key> &keys_) const
{
    if(keys.size() > 3)
        return false;

    bool has_unicode = false;
    for(const auto &key: keys)
        has_unicode |= key.isUnicode();

    key_data.options.shift_pressed = shift_state.contains(Qt::Key_Shift) && !has_unicode; // If there is any unicode char in the hot key, the "Shift" state seems to be processed in there instead and doesn't work with it specified again here
    key_data.options.control_pressed = shift_state.contains(Qt::Key_Control);
    key_data.options.alt_pressed = shift_state.contains(Qt::Key_Alt);
    key_data.options.logo_pressed = shift_state.contains(Qt::Key_Meta);
    key_data.options.menu_pressed = shift_state.contains(Qt::Key_Menu);
    key_data.options.sys_req_pressed = shift_state.contains(Qt::Key_SysReq);

    key_data.options.input_key_count = static_cast<uint32_t>(keys.size()) & 0x3;
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
        bool success = false;
        auto key = EFIKey::fromString(keys[k], &success);
        if(!success)
            return {};

        value.keys.push_back(key);
        ++k;
    }

    return value;
}

QString EFIKeySequence::toString(bool escaped) const
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

    if(escaped)
    {
        // Simplest way to escape non-printable unicode characters in Qt?
        auto repr = QJsonDocument{QJsonArray{str}}.toJson(QJsonDocument::JsonFormat::Compact);
        return repr.mid(2, repr.length() - 4);
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

bool EFIKeySequence::addKey(int key, Qt::KeyboardModifiers modifiers, const QString &text, qsizetype maxKeys)
{
    if(auto modif = std::find_if(efi_modifiers.begin(), efi_modifiers.end(), [&](const auto &row)
           { return std::get<Qt::Key>(row) == key; });
        modif != efi_modifiers.end())
    {
        auto mod = std::get<Qt::Key>(*modif);
        shift_state.insert(mod);
        if(modifiers & Qt::ShiftModifier)
            fixShiftState();

        return true;
    }

    if(keys.size() >= maxKeys)
        return false;

    bool success = false;
    EFIKey efi_key = EFIKey::fromQKey(key, modifiers, text, &success);
    if(!success)
        return false;

    if(!keys.isEmpty() && keys.last() == efi_key)
        return false;

    keys.push_back(efi_key);
    if(modifiers & Qt::ShiftModifier)
        fixShiftState();

    return true;
}

void EFIKeySequence::fixShiftState()
{
    bool has_upper = false;
    bool has_unicode = false;
    for(auto &key: keys)
    {
        has_unicode |= key.isUnicode();
        key = key.toUpper();
        has_upper |= key.isUpper();
    }

    if(!has_unicode || has_upper)
        shift_state.insert(Qt::Key_Shift);

    else // has unicode chars but no uppercase -> no Shift press necessary
        shift_state.remove(Qt::Key_Shift);
}
