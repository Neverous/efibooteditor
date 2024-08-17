// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#pragma pack(push, 1)
typedef struct efi_input_key_s
{
    uint16_t scan_code;
    char16_t unicode_char;
} efi_input_key;

typedef union efi_boot_key_data_u
{
    struct efi_boot_key_data_options_s
    {
        uint32_t revision : 8;
        uint32_t shift_pressed : 1;
        uint32_t control_pressed : 1;
        uint32_t alt_pressed : 1;
        uint32_t logo_pressed : 1;
        uint32_t menu_pressed : 1;
        uint32_t sys_req_pressed : 1;
        uint32_t reserved : 16;
        uint32_t input_key_count : 2;
    } options;
    uint32_t packed_value;
} efi_boot_key_data;

typedef struct efi_key_option_s
{
    efi_boot_key_data key_data;
    uint32_t boot_option_crc;
    uint16_t boot_option;
    efi_boot_key_data keys[ANYSIZE_ARRAY];
} efi_key_option;
#pragma pack(pop)
