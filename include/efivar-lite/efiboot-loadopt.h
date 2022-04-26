// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "efivar-dp.h"

#pragma pack(push, 1)
typedef struct efi_load_option_s
{
    uint32_t attributes;
    uint16_t file_path_list_length;
    uint16_t description[1];
    // uint8_t file_path_list[1];
    // uint8_t optional_data[1];
} efi_load_option;
#pragma pack(pop)

efidp efi_loadopt_path(efi_load_option *load_option, ssize_t size_limit) ATTR_NONNULL(1);
uint16_t efi_loadopt_pathlen(efi_load_option *load_option, ssize_t size_limit) ATTR_NONNULL(1) ATTR_VISIBILITY("default");
int efi_loadopt_optional_data(efi_load_option *load_option, size_t load_option_size, uint8_t **optional_data, size_t *optional_data_size) ATTR_NONNULL(1, 3) ATTR_VISIBILITY("default");
