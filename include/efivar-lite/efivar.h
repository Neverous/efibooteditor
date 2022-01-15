// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * Cutdown version of efivar (https://github.com/rhboot/efivar) with only
 * necessary things for boot manager manipulation.
 */
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct ATTR_ALIGN(1)
{
#ifdef _WIN32
    TCHAR data[39];
#else
    uint32_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
    uint8_t e[6];
#endif
} efi_guid_t;
#pragma pack(pop)

static const uint32_t EFI_VARIABLE_ATTRIBUTE_NON_VOLATILE = 0x00000001;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_BOOTSERVICE_ACCESS = 0x00000002;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_RUNTIME_ACCESS = 0x00000004;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_HARDWARE_ERROR_RECORD = 0x00000008;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_AUTHENTICATED_WRITE_ACCESS = 0x00000010;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS = 0x00000020;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_APPEND_WRITE = 0x00000040;
static const uint32_t EFI_VARIABLE_ATTRIBUTE_DEFAULTS = 0x00000001 | 0x00000002 | 0x00000004;

extern const efi_guid_t efi_guid_global;

extern int efi_variables_supported(void);

extern int efi_get_variable(efi_guid_t guid, const TCHAR *name, uint8_t **data, size_t *data_size, uint32_t *attributes) ATTR_NONNULL(2, 3, 4, 5);
extern int efi_del_variable(efi_guid_t guid, const TCHAR *name) ATTR_NONNULL(2);
extern int efi_set_variablex(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes) ATTR_NONNULL(2, 3);
extern int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name) ATTR_NONNULL(1, 2);

extern int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b);

extern int efi_error_get(unsigned int n, TCHAR **const filename, TCHAR **const function, int *line, TCHAR **const message, int *error) ATTR_NONNULL(2, 3, 4, 5, 6);

#include "efivar-dp.h"
