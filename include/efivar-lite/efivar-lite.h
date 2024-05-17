// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * Cutdown version of efivar (https://github.com/rhboot/efivar) with only
 * necessary things for boot manager manipulation.
 */
#pragma once

#include <stdarg.h>
#include <string.h>

#include "compat.h"

#pragma pack(push, 1)
typedef struct ATTR_ALIGN(1)
{
#if defined(_WIN32)
    TCHAR data[40];
#elif defined(__APPLE__)
    char data[40];
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
static const uint32_t EFI_VARIABLE_ATTRIBUTE_DEFAULTS =
#if defined(__APPLE__)
    // macOS doesn't support attributes for variables
    0
#else
    0x00000007 // EFI_VARIABLE_ATTRIBUTE_NON_VOLATILE | EFI_VARIABLE_ATTRIBUTE_BOOTSERVICE_ACCESS | EFI_VARIABLE_ATTRIBUTE_RUNTIME_ACCESS
#endif
    ;

static const mode_t EFI_VARIABLE_MODE_DEFAULTS =
#if defined(_WIN32)
    // Windows doesn't support file mode setting for variables
    STATIC_CAST(mode_t)(0)
#else
    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#endif
    ;

static const uint64_t EFI_OS_INDICATIONS_BOOT_TO_FW_UI = 0x0000000000000001;
static const uint64_t EFI_OS_INDICATIONS_TIMESTAMP_REVOCATION = 0x0000000000000002;
static const uint64_t EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED = 0x0000000000000004;
static const uint64_t EFI_OS_INDICATIONS_FMP_CAPSULE_SUPPORTED = 0x0000000000000008;
static const uint64_t EFI_OS_INDICATIONS_CAPSULE_RESULT_VAR_SUPPORTED = 0x0000000000000010;
static const uint64_t EFI_OS_INDICATIONS_START_OS_RECOVERY = 0x0000000000000020;
static const uint64_t EFI_OS_INDICATIONS_START_PLATFORM_RECOVERY = 0x0000000000000040;
static const uint64_t EFI_OS_INDICATIONS_JSON_CONFIG_DATA_REFRESH = 0x0000000000000080;

static const uint32_t EFI_BOOT_OPTION_SUPPORT_KEY = 0x00000001;
static const uint32_t EFI_BOOT_OPTION_SUPPORT_APP = 0x00000002;
static const uint32_t EFI_BOOT_OPTION_SUPPORT_SYSPREP = 0x00000010;
static const uint32_t EFI_BOOT_OPTION_SUPPORT_COUNT = 0x00000300;

extern const efi_guid_t efi_guid_global;
extern const efi_guid_t efi_guid_apple;

int efi_variables_supported(void);

int efi_get_variable(efi_guid_t guid, const TCHAR *name, uint8_t **data, size_t *data_size, uint32_t *attributes) ATTR_NONNULL(2, 3, 4, 5);
int efi_del_variable(efi_guid_t guid, const TCHAR *name) ATTR_NONNULL(2);
int efi_set_variable(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes, mode_t mode) ATTR_NONNULL(2, 3);
int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name) ATTR_NONNULL(1, 2);
void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t));

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b);

int efi_error_get(unsigned int n, TCHAR **const filename, TCHAR **const function, int *line, TCHAR **const message, int *error) ATTR_NONNULL(2, 3, 4, 5, 6);
void efi_error_clear(void);
