// SPDX-License-Identifier: LGPL-3.0-or-later

#include "efivar-lite/efivar-lite.h"

#if BYTE_ORDER == LITTLE_ENDIAN
const efi_guid_t efi_guid_apple = {0x7c436110, 0xab2a, 0x4bbb, 0x80a8, {0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};
#else
const efi_guid_t efi_guid_apple = {0x7c436110, 0xab2a, 0x4bbb, 0xa880, {0xfe, 0x41, 0x99, 0x5c, 0x9f, 0x82}};
#endif

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    (void)progress_cb;
    // Nothing to do here, efivar doesn't expose progress tracking (and it's not really needed, variables listing is pretty fast)
}
