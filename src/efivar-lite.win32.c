// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> WinAPI translation.
 */
#include "compat.h"
#include "efivar-lite/efiboot-loadopt.h"
#include "efivar-lite/efivar.h"
#include <Windows.h>

#pragma comment(lib, "advapi32.lib")

const efi_guid_t efi_guid_global = {_T("{8be4df61-93ca-11d2-aa0d-00e098032b8c}")};

int efi_variables_supported(void)
{
    LUID luid;
    if(LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &luid))
    {
        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        HANDLE process = GetCurrentProcess();
        HANDLE token;
        if(!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token))
            return 0;

        if(!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL))
            return 0;
    }
    else
        return 0;

    uint8_t *data = NULL;
    size_t data_size = 0;
    uint32_t attributes = 0;
    efi_guid_t guid = {_T("{00000000-0000-0000-0000-000000000000}")};
    if(efi_get_variable(guid, _T(""), &data, &data_size, &attributes) < 0 && GetLastError() == ERROR_INVALID_FUNCTION)
        return 0;

    return 1;
}

static uint8_t *variable_data_buffer = NULL;

int efi_get_variable(efi_guid_t guid, const TCHAR *name, uint8_t **data, size_t *data_size, uint32_t *attributes)
{
    *data_size = 64;
    DWORD attr = 0;
    DWORD ret = 0;
    do
    {
        *data_size *= 2;
        free(variable_data_buffer);
        variable_data_buffer = malloc(*data_size);
        if(!variable_data_buffer)
            return -1;

        ret = GetFirmwareEnvironmentVariableEx(name, guid.data, variable_data_buffer, (DWORD)*data_size, &attr);
    } while(ret == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    *data_size = ret;
    if(ret == 0)
        return -1;

    *attributes = attr;
    *data = variable_data_buffer;
    return 0;
}

int efi_del_variable(efi_guid_t guid, const TCHAR *name)
{
    // FIXME: Can't find an official way to do this ?
    BOOL ret = SetFirmwareEnvironmentVariableEx(name, guid.data, NULL, 0, EFI_VARIABLE_ATTRIBUTE_DEFAULTS);
    return ret == 0 ? -1 : 0;
}

int efi_set_variablex(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes)
{
    BOOL ret = SetFirmwareEnvironmentVariableEx(name, guid.data, data, (DWORD)data_size, attributes);
    return ret == 0 ? -1 : 0;
}

// UEFI Specification, Version 2.8
static const TCHAR *variable_names[] = {
    _T("AuditMode"),
    _T("BootCurrent"),
    _T("BootNext"),
    _T("BootOrder"),
    _T("BootOptionSupport"),
    _T("ConIn"),
    _T("ConInDev"),
    _T("ConOut"),
    _T("ConOutDev"),
    _T("dbDefault"),
    _T("dbrDefault"),
    _T("dbtDefault"),
    _T("dbxDefault"),
    _T("DeployedMode"),
    _T("DriverOrder"),
    _T("ErrOut"),
    _T("ErrOutDev"),
    _T("HwErrRecSupprot"),
    _T("KEK"),
    _T("KEKDefault"),
    _T("Lang"),
    _T("LangCodes"),
    _T("OsIndications"),
    _T("OsIndicationsSupported"),
    _T("OsRecoveryOrder"),
    _T("PK"),
    _T("PKDefault"),
    _T("PlatformLangCodes"),
    _T("PlatformLang"),
    _T("RuntimeServicesSupported"),
    _T("SignatureSupport"),
    _T("SecureBoot"),
    _T("SetupMode"),
    _T("SysPrepOrder"),
    _T("Timeout"),
    _T("VendorKeys"),
};

static const TCHAR *enumerated_variable_names[] = {
    _T("Boot"),
    _T("Driver"),
    _T("Key"),
    _T("PlatformRecovery"),
    _T("SysPrep"),
};

static TCHAR variable_name_buffer[32];

static size_t current_variable = 0;

static int _efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
{
    *guid = NULL;
    *name = NULL;
    size_t index = current_variable;
    if(index < sizeof(variable_names) / sizeof(variable_names[0]))
    {
        *guid = (efi_guid_t *)&efi_guid_global;
        *name = (TCHAR *)variable_names[index];
        ++current_variable;
        return 1;
    }

    index -= sizeof(variable_names) / sizeof(variable_names[0]);
    if(index / 65536 < sizeof(enumerated_variable_names) / sizeof(enumerated_variable_names[0]))
    {
        size_t enum_index = index / 65536;
        size_t enum_value = index - enum_index * 65536;
        *guid = (efi_guid_t *)&efi_guid_global;
        _sntprintf_s(variable_name_buffer, 32, 31, _T("%s%04zx"), enumerated_variable_names[enum_index], enum_value);
        *name = (TCHAR *)&variable_name_buffer;
        ++current_variable;
        return 1;
    }

    index -= sizeof(enumerated_variable_names) / sizeof(enumerated_variable_names[0]) * 65536;
    if(index == 0)
    {
        current_variable = 0;
        return 0;
    }

    return -1;
}

int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
{
    while(1)
    {
        int ret = _efi_get_next_variable_name(guid, name);
        if(ret <= 0)
            return ret;

        if(GetFirmwareEnvironmentVariable(*name, (*guid)->data, NULL, 0) == 0)
        {
            DWORD err = GetLastError();
            if(err != ERROR_INSUFFICIENT_BUFFER)
                continue;
        }

        return ret;
    }
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return memcmp(a, b, sizeof(efi_guid_t));
}

static TCHAR error_buffer[1024];

int efi_error_get(unsigned int n, TCHAR **const filename, TCHAR **const function, int *line, TCHAR **const message, int *error)
{
    if(n == 1)
        return 0;

    if(n > 1)
        return -1;

    *filename = _T("windows.h");
    *line = -1;
    DWORD err = GetLastError();
    *error = (int)err;
    *function = _T("WINAPI");
    if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_buffer, 1024, NULL))
        return -1;

    *message = error_buffer;
    return 1;
}

efidp efi_loadopt_path(efi_load_option *opt, ssize_t limit)
{
    uint8_t *ptr = (uint8_t *)opt;
    if((size_t)limit <= offsetof(efi_load_option, description))
        return NULL;

    limit -= offsetof(efi_load_option, description);
    ptr += offsetof(efi_load_option, description);
    for(size_t d = 0; limit > 0 && opt->description[d]; ++d, limit -= sizeof(opt->description[0]), ptr += sizeof(opt->description[0]))
        ;
    // \0
    limit -= sizeof(opt->description[0]);
    ptr += sizeof(opt->description[0]);
    if(limit == 0)
        return NULL;

    if(limit < opt->file_path_list_length)
        return NULL;

    return (efidp)ptr;
}

uint16_t efi_loadopt_pathlen(efi_load_option *opt, ssize_t limit)
{
    uint16_t len = opt->file_path_list_length;
    if(limit >= 0)
    {
        if(len > limit)
            return 0;

        if((size_t)limit - offsetof(efi_load_option, file_path_list_length) < len)
            return 0;
    }

    return len;
}

int efi_loadopt_optional_data(efi_load_option *opt, size_t opt_size, unsigned char **datap, size_t *len)
{
    uint8_t *ptr = (uint8_t *)efi_loadopt_path(opt, (ssize_t)opt_size);
    if(!ptr)
        return -1;

    ptr += opt->file_path_list_length;
    opt_size -= (size_t)(ptr - (uint8_t *)opt);
    *len = opt_size;
    *datap = ptr;
    return 0;
}
