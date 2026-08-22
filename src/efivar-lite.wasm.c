// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * WASM doesn't support reading EFI variables from the system.
 */
#include "efivar-lite.common.h"

#if BYTE_ORDER == LITTLE_ENDIAN
const efi_guid_t efi_guid_global = {0x8BE4DF61, 0x93CA, 0x11D2, 0x0daa, {0x00, 0xe0, 0x98, 0x03, 0x2B, 0x8c}};
const efi_guid_t efi_guid_apple = {0x7c436110, 0xab2a, 0x4bbb, 0x80a8, {0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};
#else
const efi_guid_t efi_guid_global = {0x8BE4DF61, 0x93CA, 0x11D2, 0xaa0d, {0x00, 0xe0, 0x98, 0x03, 0x2B, 0x8c}};
const efi_guid_t efi_guid_apple = {0x7c436110, 0xab2a, 0x4bbb, 0xa880, {0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};
#endif

int efi_variables_supported(void)
{
    return 0;
}

int efi_get_variable(efi_guid_t guid, const TCHAR *name, uint8_t **data, size_t *data_size, uint32_t *attributes)
{
    (void)guid;
    (void)name;
    (void)data;
    (void)data_size;
    (void)attributes;
    return -1;
}

int efi_del_variable(efi_guid_t guid, const TCHAR *name)
{
    (void)guid;
    (void)name;
    return -1;
}

int efi_set_variable(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes, mode_t mode)
{
    (void)guid;
    (void)name;
    (void)data;
    (void)data_size;
    (void)attributes;
    (void)mode;
    return -1;
}

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    (void)progress_cb;
}

int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
{
    (void)guid;
    (void)name;
    return -1;
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return memcmp(a, b, sizeof(efi_guid_t));
}

int efi_error_get(unsigned int n, TCHAR **const filename, TCHAR **const function, int *line, TCHAR **const message, int *error)
{
    (void)n;
    (void)filename;
    (void)function;
    (void)line;
    (void)message;
    (void)error;
    return -1;
}

void efi_error_clear(void)
{
    // Nothing to do
}
