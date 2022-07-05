// SPDX-License-Identifier: LGPL-3.0-or-later
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include "compat.h"
#include "efivar-lite/efivar.h"

extern const size_t EFI_MAX_VARIABLES;
int _efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name);
