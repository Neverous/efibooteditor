// SPDX-License-Identifier: LGPL-3.0-or-later
#include <efivar/efivar.h>

int efi_set_variablex(efi_guid_t guid, const char *name, uint8_t *data, size_t data_size, uint32_t attributes)
{
    return efi_set_variable(guid, name, data, data_size, attributes, S_IRWXU | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR);
}
