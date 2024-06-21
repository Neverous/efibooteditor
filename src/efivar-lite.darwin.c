// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> IOKit translation.
 */
#include <IOKit/IOKitLib.h>
#include <mach/mach_error.h>

#include "compat.h"
#include "efivar-lite.common.h"
#include "efivar-lite/efivar-lite.h"
#include "efivar-lite/load-option.h"

const efi_guid_t efi_guid_global = {"8BE4DF61-93CA-11D2-AA0D-00E098032B8C"};
const efi_guid_t efi_guid_apple = {"7C436110-AB2A-4BBB-A880-FE41995C9F82"};

static io_registry_entry_t options_entry;
static kern_return_t err;
static char *last_iokit_function = NULL;

int efi_variables_supported(void)
{
    options_entry = IORegistryEntryFromPath(MACH_PORT_NULL, "IODeviceTree:/options");
    if(!options_entry)
    {
        last_iokit_function = "IORegistryEntryFromPath";
        return 0;
    }

    return 1;
}

static uint8_t *variable_data_buffer = NULL;

static CFStringRef _get_nvram_variable_name(const efi_guid_t *guid, const char *name)
{
    CFMutableStringRef name_cf = CFStringCreateMutable(kCFAllocatorDefault, 0);
    if(name_cf == NULL)
    {
        last_iokit_function = "CFStringCreateMutable";
        return NULL;
    }

    CFStringAppendCString(name_cf, guid->data, kCFStringEncodingUTF8);
    CFStringAppendCString(name_cf, ":", kCFStringEncodingUTF8);
    CFStringAppendCString(name_cf, name, kCFStringEncodingUTF8);
    return name_cf;
}

int efi_get_variable(efi_guid_t guid, const char *name, uint8_t **data, size_t *data_size, uint32_t *attributes)
{
    CFStringRef name_cf = _get_nvram_variable_name(&guid, name);
    if(name_cf == NULL)
        return -1;

    CFTypeRef value_cf = IORegistryEntryCreateCFProperty(options_entry, name_cf, kCFAllocatorDefault, 0);
    CFRelease(name_cf);
    if(value_cf == NULL)
    {
        last_iokit_function = "IORegistryEntryCreateCFProperty";
        return -1;
    }

    *data_size = (size_t)CFDataGetLength(value_cf);
    free(variable_data_buffer);
    variable_data_buffer = malloc(*data_size);
    if(!variable_data_buffer)
    {
        last_iokit_function = NULL;
        CFRelease(value_cf);
        return -1;
    }

    const void *ret = memcpy(variable_data_buffer, CFDataGetBytePtr(value_cf), *data_size);
    CFRelease(value_cf);
    if(ret == NULL)
    {
        last_iokit_function = NULL;
        return -1;
    }

    *data = variable_data_buffer;
    *attributes = 0;
    return 0;
}

int efi_del_variable(efi_guid_t guid, const char *name)
{
    return efi_set_variable(guid, kIONVRAMDeletePropertyKey, (uint8_t *)name, strlen(name), 0, 0);
}

int efi_set_variable(efi_guid_t guid, const char *name, uint8_t *data, size_t data_size, uint32_t attributes, mode_t mode)
{
    (void)attributes;
    (void)mode;

    CFStringRef name_cf = _get_nvram_variable_name(&guid, name);
    if(name_cf == NULL)
        return -1;

    CFDataRef value_cf = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, data, (CFIndex)data_size, kCFAllocatorDefault);
    if(value_cf == NULL)
    {
        last_iokit_function = "CFDataCreateWithBytesNoCopy";
        CFRelease(name_cf);
        return -1;
    }

    err = IORegistryEntrySetCFProperty(options_entry, name_cf, value_cf);
    CFRelease(value_cf);
    CFRelease(name_cf);
    if(err != KERN_SUCCESS)
    {
        last_iokit_function = "IORegistryEntrySetCFProperty";
        return -1;
    }

    return 0;
}

static void (*efi_get_next_variable_name_progress_cb)(size_t, size_t) = NULL;

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    efi_get_next_variable_name_progress_cb = progress_cb;
}

static size_t current_variable = 0;
int efi_get_next_variable_name(efi_guid_t **guid, char **name)
{
    while(1)
    {
        ++current_variable;
        if(efi_get_next_variable_name_progress_cb && current_variable % (EFI_MAX_VARIABLES / 100u) == 0u)
            efi_get_next_variable_name_progress_cb(current_variable, EFI_MAX_VARIABLES);

        int ret = _efi_get_next_variable_name(guid, name);
        if(ret <= 0)
        {
            current_variable = 0u;
            return ret;
        }

        CFStringRef name_cf = _get_nvram_variable_name(*guid, *name);
        if(name_cf == NULL)
            return -1;

        CFTypeRef value_cf = IORegistryEntryCreateCFProperty(options_entry, name_cf, kCFAllocatorDefault, 0);
        CFRelease(name_cf);
        if(value_cf == NULL)
            continue;

        CFIndex length = CFDataGetLength(value_cf);
        CFRelease(value_cf);
        if(length > 0)
            return ret;
    }
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return strncmp(a->data, b->data, sizeof(a->data) / sizeof(a->data[0]));
}

int efi_guid_to_str(const efi_guid_t *guid, char **sp)
{
    *sp = (char *)guid->data;
    return 0;
}

int efi_error_get(unsigned int n, char **const filename, char **const function, int *line, char **const message, int *error)
{
    if(n == 1u)
        return 0;

    if(n > 1u)
        return -1;

    *filename = "IOKitLib.h";
    *line = -1;
    *error = errno; // FIXME
    *function = last_iokit_function;
    *message = mach_error_string(err);
    return 1;
}

void efi_error_clear(void)
{
    // Nothing to do
}
