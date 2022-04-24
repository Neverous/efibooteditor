// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> IOKit translation.
 */
#include <IOKit/IOKitLib.h>
#include <mach/mach_error.h>

#include "compat.h"
#include "efivar-lite/efiboot-loadopt.h"
#include "efivar-lite/efivar.h"

const efi_guid_t efi_guid_global = {"8BE4DF61-93CA-11D2-AA0D-00E098032B8C"};

static io_registry_entry_t options_entry;
static kern_return_t err;
static char *last_iokit_function = NULL;

int efi_variables_supported(void)
{
    options_entry = IORegistryEntryFromPath(kIOMasterPortDefault, "IODeviceTree:/options");
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

    *data_size = CFDataGetLength(value_cf);
    free(variable_data_buffer);
    variable_data_buffer = malloc(*data_size);
    if(!variable_data_buffer)
    {
        last_iokit_function = NULL;
        CFRelease(value_cf);
        return -1;
    }

    void *ret = memcpy(variable_data_buffer, CFDataGetBytePtr(value_cf), *data_size);
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
    CFStringRef name_cf = _get_nvram_variable_name(&guid, name);
    if(name_cf == NULL)
        return -1;

    CFDataRef value_cf = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, data, data_size, kCFAllocatorDefault);
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

int _efi_get_next_variable_name(efi_guid_t **guid, char **name);

int efi_get_next_variable_name(efi_guid_t **guid, char **name)
{
    while(1)
    {
        int ret = _efi_get_next_variable_name(guid, name);
        if(ret <= 0)
            return ret;

        CFStringRef name_cf = _get_nvram_variable_name(*guid, *name);
        if(name_cf == NULL)
            return -1;

        CFTypeRef value_cf = IORegistryEntryCreateCFProperty(options_entry, name_cf, kCFAllocatorDefault, 0);
        CFRelease(name_cf);
        if(value_cf == NULL)
            continue;

        size_t length = CFDataGetLength(value_cf);
        CFRelease(value_cf);
        if(length > 0)
            return ret;
    }
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return memcmp(a, b, sizeof(efi_guid_t));
}

int efi_error_get(unsigned int n, char **const filename, char **const function, int *line, char **const message, int *error)
{
    if(n == 1)
        return 0;

    if(n > 1)
        return -1;

    *filename = "IOKitLib.h";
    *line = -1;
    *error = (int)errno; // FIXME
    *function = last_iokit_function;
    *message = mach_error_string(err);
    return 1;
}
