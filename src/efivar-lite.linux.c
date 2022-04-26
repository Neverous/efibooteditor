// SPDX-License-Identifier: LGPL-3.0-or-later

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(int, int))
{
    (void)progress_cb;
    // Nothing to do here, efivar doesn't expose progress tracking (and it's not really needed, variables listing is pretty fast)
}
