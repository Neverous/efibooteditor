// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> IOKit translation.
 */
#include "efivar-lite.common.h"

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach_error.h>

#include "efivar-lite/device-paths.h"
#include "efivar-lite/load-option.h"

const efi_guid_t efi_guid_global = {"8BE4DF61-93CA-11D2-AA0D-00E098032B8C"};
const efi_guid_t efi_guid_apple = {"7C436110-AB2A-4BBB-A880-FE41995C9F82"};

static io_registry_entry_t options_entry;
static kern_return_t err;
static char *last_iokit_function = nullptr;

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

static uint8_t *variable_data_buffer = nullptr;

static CFStringRef get_nvram_variable_name(const efi_guid_t *guid, const char *name)
{
    CFMutableStringRef name_cf = CFStringCreateMutable(kCFAllocatorDefault, 0);
    if(name_cf == nullptr)
    {
        last_iokit_function = "CFStringCreateMutable";
        return nullptr;
    }

    CFStringAppendCString(name_cf, guid->data, kCFStringEncodingUTF8);
    CFStringAppendCString(name_cf, ":", kCFStringEncodingUTF8);
    CFStringAppendCString(name_cf, name, kCFStringEncodingUTF8);
    return name_cf;
}

int efi_get_variable(efi_guid_t guid, const char *name, uint8_t **data, size_t *data_size, uint32_t *attributes)
{
    CFStringRef name_cf = get_nvram_variable_name(&guid, name);
    if(name_cf == nullptr)
    {
        last_iokit_function = nullptr;
        return -1;
    }

    CFTypeRef value_cf = IORegistryEntryCreateCFProperty(options_entry, name_cf, kCFAllocatorDefault, kNilOptions);
    CFRelease(name_cf);
    if(value_cf == nullptr)
    {
        last_iokit_function = "IORegistryEntryCreateCFProperty";
        return -1;
    }

    if(CFGetTypeID(value_cf) == CFDataGetTypeID())
        *data_size = (size_t)CFDataGetLength(value_cf);

    else if(CFGetTypeID(value_cf) == CFStringGetTypeID())
        *data_size = (size_t)CFStringGetLength(value_cf) + 1;

    else if(CFGetTypeID(value_cf) == CFNumberGetTypeID())
        *data_size = sizeof(int32_t);

    else if(CFGetTypeID(value_cf) == CFBooleanGetTypeID())
        *data_size = sizeof(bool);

    else
    {
        last_iokit_function = "CFGetTypeID";
        CFRelease(value_cf);
        return -1;
    }

    free(variable_data_buffer);
    variable_data_buffer = malloc(*data_size);
    if(variable_data_buffer == nullptr)
    {
        last_iokit_function = nullptr;
        CFRelease(value_cf);
        return -1;
    }

    const void *ret = nullptr;
    if(CFGetTypeID(value_cf) == CFDataGetTypeID())
        ret = memcpy(variable_data_buffer, CFDataGetBytePtr(value_cf), *data_size);

    else if(CFGetTypeID(value_cf) == CFStringGetTypeID())
    {
        if(CFStringGetCString(value_cf, (char *)variable_data_buffer, (CFIndex)*data_size, kCFStringEncodingUTF8))
            ret = variable_data_buffer;
    }
    else if(CFGetTypeID(value_cf) == CFNumberGetTypeID())
    {
        if(CFNumberGetValue(value_cf, kCFNumberSInt32Type, variable_data_buffer))
            ret = variable_data_buffer;
    }
    else if(CFGetTypeID(value_cf) == CFBooleanGetTypeID())
    {
        *variable_data_buffer = CFBooleanGetValue(value_cf);
        ret = variable_data_buffer;
    }
    else
    {
        last_iokit_function = "CFGetTypeID";
        CFRelease(value_cf);
        return -1;
    }

    CFRelease(value_cf);
    if(ret == nullptr)
    {
        last_iokit_function = nullptr;
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

    CFStringRef name_cf = get_nvram_variable_name(&guid, name);
    if(name_cf == nullptr)
        return -1;

    CFTypeRef value_cf = IORegistryEntryCreateCFProperty(options_entry, name_cf, kCFAllocatorDefault, kNilOptions);
    if(value_cf == nullptr)
    {
        last_iokit_function = "IORegistryEntryCreateCFProperty";
        return -1;
    }

    if(CFGetTypeID(value_cf) == CFDataGetTypeID())
    {
        CFRelease(value_cf);
        value_cf = CFDataCreate(kCFAllocatorDefault, data, (CFIndex)data_size);
        last_iokit_function = "CFDataCreate";
    }
    else if(CFGetTypeID(value_cf) == CFStringGetTypeID())
    {
        CFRelease(value_cf);
        value_cf = CFStringCreateWithBytes(kCFAllocatorDefault, data, (CFIndex)data_size, kCFStringEncodingUTF8, false);
        last_iokit_function = "CFStringCreateWithBytes";
    }
    else if(CFGetTypeID(value_cf) == CFNumberGetTypeID())
    {
        CFRelease(value_cf);
        if(data_size == sizeof(int32_t))
            value_cf = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, data);
        else
            value_cf = nullptr;

        last_iokit_function = "CFNumberCreate";
    }
    else if(CFGetTypeID(value_cf) == CFBooleanGetTypeID())
    {
        CFRelease(value_cf);
        if(data_size == sizeof(bool))
            value_cf = (*(bool *)data) ? kCFBooleanTrue : kCFBooleanFalse;
        else
            value_cf = nullptr;

        last_iokit_function = "CFBooleanCreate";
    }
    else
    {
        last_iokit_function = "CFGetTypeID";
        CFRelease(value_cf);
        CFRelease(name_cf);
        return -1;
    }

    if(value_cf == nullptr)
    {
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

static void (*efi_get_next_variable_name_progress_cb)(size_t, size_t) = nullptr;

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    efi_get_next_variable_name_progress_cb = progress_cb;
}

static CFMutableDictionaryRef variables_cf = nullptr;
static size_t current_variable = 0;
static size_t total_variables = 0;
static CFTypeRef *variable_names = nullptr;
static char *variable_name_buffer = nullptr;
static efi_guid_t current_guid = {0};

int efi_get_next_variable_name(efi_guid_t **guid, char **name)
{
    if(current_variable == 0)
    {
        err = IORegistryEntryCreateCFProperties(options_entry, &variables_cf, kCFAllocatorDefault, kNilOptions);
        if(err != KERN_SUCCESS)
        {
            last_iokit_function = "IORegistryEntryCreateCFProperties";
            return -1;
        }

        total_variables = (size_t)CFDictionaryGetCount(variables_cf);
        free(variable_names);
        variable_names = malloc(total_variables * sizeof(CFTypeRef));
        if(variable_names == nullptr)
        {
            CFRelease(variables_cf);
            last_iokit_function = nullptr;
            return -1;
        }

        CFDictionaryGetKeysAndValues(variables_cf, variable_names, nullptr);
    }

    if(efi_get_next_variable_name_progress_cb)
        efi_get_next_variable_name_progress_cb(current_variable, total_variables);

    if(current_variable >= total_variables)
    {
        current_variable = 0;
        CFRelease(variables_cf);
        return 0;
    }

    if(CFGetTypeID(variable_names[current_variable]) != CFStringGetTypeID())
    {
        CFRelease(variables_cf);
        last_iokit_function = "CFGetTypeID";
        return -1;
    }

    CFStringRef variable = (CFStringRef)variable_names[current_variable++];
    size_t variable_name_length = (size_t)CFStringGetLength(variable) + 1;
    free(variable_name_buffer);
    variable_name_buffer = malloc(variable_name_length);
    if(variable_name_buffer == nullptr)
    {
        CFRelease(variables_cf);
        last_iokit_function = nullptr;
        return -1;
    }

    if(!CFStringGetCString(variable, variable_name_buffer, (CFIndex)variable_name_length, kCFStringEncodingUTF8))
    {
        CFRelease(variables_cf);
        last_iokit_function = "CFStringGetCString";
        return -1;
    }

    char *ptr = strchr(variable_name_buffer, ':');
    if(ptr == nullptr)
    {
        *guid = (efi_guid_t *)&efi_guid_apple;
        *name = variable_name_buffer;
        return 1;
    }

    if(ptr - variable_name_buffer != 36)
    {
        *guid = (efi_guid_t *)&efi_guid_apple;
        *name = variable_name_buffer;
        return 1;
    }

    if(memcpy(current_guid.data, variable_name_buffer, 36) == nullptr)
    {
        CFRelease(variables_cf);
        last_iokit_function = nullptr;
        return -1;
    }

    *guid = &current_guid;
    *name = ptr + 1;
    return 1;
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return strncmp(a->data, b->data, sizeof(a->data) / sizeof(a->data[0]));
}

int efi_error_get(unsigned int n, char **const filename, char **const function, int *line, char **const message, int *error)
{
    if(n == 1u)
        return 0;

    if(n > 1u)
        return -1;

    *filename = "IOKitLib.h";
    *line = -1;
    *error = err;
    *function = last_iokit_function;
    *message = mach_error_string(err);
    return 1;
}

void efi_error_clear(void)
{
    err = KERN_SUCCESS;
    last_iokit_function = nullptr;
}
