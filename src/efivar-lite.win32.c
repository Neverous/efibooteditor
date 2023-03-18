// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> WinAPI translation.
 */
#include <Windows.h>

#include "compat.h"
#include "efivar-lite.common.h"
#include "efivar-lite/efiboot-loadopt.h"
#include "efivar-lite/efivar.h"

#pragma comment(lib, "advapi32.lib")

const efi_guid_t efi_guid_global = {_T("{8be4df61-93ca-11d2-aa0d-00e098032b8c}")};
const efi_guid_t efi_guid_apple = {_T("{7c436110-ab2a-4bbb-a880-fe41995c9f82}")};
static const TCHAR *last_winapi_function = NULL;

int efi_variables_supported(void)
{
    LUID luid;
    if(LookupPrivilegeValue(NULL, SE_SYSTEM_ENVIRONMENT_NAME, &luid))
    {
        TOKEN_PRIVILEGES tp;
        memset(&tp, 0, sizeof(tp));
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        HANDLE process = GetCurrentProcess();
        HANDLE token;
        if(!OpenProcessToken(process, TOKEN_ADJUST_PRIVILEGES, &token))
        {
            last_winapi_function = _T("OpenProcessToken");
            return 0;
        }

        if(!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL))
        {
            last_winapi_function = _T("AdjustTokenPrivileges");
            return 0;
        }
    }
    else
    {
        last_winapi_function = _T("LookupPrivilegeValue");
        return 0;
    }

    FIRMWARE_TYPE firmware_type;
    memset(&firmware_type, 0, sizeof(firmware_type));
    if(!GetFirmwareType(&firmware_type) || firmware_type != FirmwareTypeUefi)
    {
        last_winapi_function = _T("GetFirmwareType");
        return 0;
    }

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
        last_winapi_function = _T("GetFirmwareEnvironmentVariableEx");
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
    // setting nSize (data_size) = 0 => deletes variable
    // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setfirmwareenvironmentvariableexa#parameters
    return efi_set_variable(guid, name, NULL, 0, 0, 0);
}

int efi_set_variable(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes, mode_t mode)
{
    (void)mode;
    BOOL ret = SetFirmwareEnvironmentVariableEx(name, guid.data, data, (DWORD)data_size, attributes);
    last_winapi_function = _T("SetFirmwareEnvironmentVariableEx");
    return ret == 0 ? -1 : 0;
}

static void (*efi_get_next_variable_name_progress_cb)(size_t, size_t) = NULL;

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    efi_get_next_variable_name_progress_cb = progress_cb;
}

static size_t current_variable = 0u;
int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
{
    while(1)
    {
        if(efi_get_next_variable_name_progress_cb)
            efi_get_next_variable_name_progress_cb(current_variable++, EFI_MAX_VARIABLES);

        int ret = _efi_get_next_variable_name(guid, name);
        if(ret <= 0)
        {
            current_variable = 0u;
            return ret;
        }

        if(GetFirmwareEnvironmentVariable(*name, (*guid)->data, NULL, 0) == 0)
        {
            last_winapi_function = _T("GetFirmwareEnvironmentVariable");
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
    if(n == 1u)
        return 0;

    if(n > 1u)
        return -1;

    *filename = _T("windows.h");
    *line = -1;
    DWORD err = GetLastError();
    *error = (int)err;
    *function = (TCHAR *)last_winapi_function;
    if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_buffer, 1024, NULL))
        return -1;

    *message = error_buffer;
    return 1;
}
