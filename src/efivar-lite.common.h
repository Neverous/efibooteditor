// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <stdarg.h>
#include <string.h>

#include "compat.h"
#include "efivar-lite/efivar.h"

extern const size_t EFI_MAX_VARIABLES;
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
int _efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
