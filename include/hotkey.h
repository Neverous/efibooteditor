// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QMetaType>

#include "efiboot.h"
#include "efikeysequence.h"

class HotKey
{
public:
    int index = -1;
    uint16_t boot_option = 0;
    EFIKeySequence keys = {};
    QByteArray vendor_data = {};
    uint32_t efi_attributes = EFIBoot::EFI_VARIABLE_ATTRIBUTE_DEFAULTS;
    QString error = {};

    bool is_error = false;

public:
    static HotKey fromEFIBootKeyOption(const EFIBoot::Key_option &key_option);
    static HotKey fromError(const QString &error);
    EFIBoot::Key_option toEFIBootKeyOption(const std::unordered_map<uint16_t, uint32_t> &crc32) const;

    static std::optional<HotKey> fromJSON(const QJsonObject &obj, qsizetype maxKeys);
    QJsonObject toJSON() const;
};
