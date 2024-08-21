// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * efivar interface <> WinAPI translation.
 */
#include "efivar-lite.common.h"

#include <Windows.h>
#include <ntstatus.h>
#include <winternl.h>

#include "efivar-lite/device-paths.h"
#include "efivar-lite/load-option.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ntdll.lib")

// NT syscall for fetching EFI Variables
NTSTATUS
NTAPI
NtEnumerateSystemEnvironmentValuesEx(
    _In_ ULONG InformationClass,
    _Out_ PVOID Buffer,
    _Inout_ PULONG BufferLength);

static const ULONG SystemEnvironmentNameInformation = 1;
typedef struct
{
    ULONG NextEntryOffset;
    GUID VendorGUID;
    TCHAR Name[ANYSIZE_ARRAY];
} variable_info_t;
// END

const efi_guid_t efi_guid_global = {_T("{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}")};
const efi_guid_t efi_guid_apple = {_T("{7C436110-AB2A-4BBB-A880-FE41995C9F82}")};
static TCHAR *last_winapi_function = nullptr;

int efi_variables_supported(void)
{
    LUID luid;
    if(LookupPrivilegeValue(nullptr, SE_SYSTEM_ENVIRONMENT_NAME, &luid))
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

        if(!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr))
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

    uint8_t *data = nullptr;
    size_t data_size = 0;
    uint32_t attributes = 0;
    efi_guid_t guid = {_T("{00000000-0000-0000-0000-000000000000}")};
    if(efi_get_variable(guid, _T(""), &data, &data_size, &attributes) < 0 && GetLastError() == ERROR_INVALID_FUNCTION)
        return 0;

    return 1;
}

static uint8_t *variable_data_buffer = nullptr;

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
    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setfirmwareenvironmentvariablea#parameters
    BOOL ret = SetFirmwareEnvironmentVariable(name, guid.data, nullptr, 0);
    last_winapi_function = _T("SetFirmwareEnvironmentVariable");
    return ret == 0 ? -1 : 0;
}

int efi_set_variable(efi_guid_t guid, const TCHAR *name, uint8_t *data, size_t data_size, uint32_t attributes, mode_t mode)
{
    (void)mode;
    BOOL ret = SetFirmwareEnvironmentVariableEx(name, guid.data, data, (DWORD)data_size, attributes);
    last_winapi_function = _T("SetFirmwareEnvironmentVariableEx");
    return ret == 0 ? -1 : 0;
}

static void (*efi_get_next_variable_name_progress_cb)(size_t, size_t) = nullptr;

void efi_set_get_next_variable_name_progress_cb(void (*progress_cb)(size_t, size_t))
{
    efi_get_next_variable_name_progress_cb = progress_cb;
}

static ULONG current_offset = 0u;
static PVOID variables = nullptr;
static ULONG variables_size = 0u;
static efi_guid_t current_guid = {0};

int efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
{
    if(current_offset == 0u)
    {
        NTSTATUS ret = 0;
        variables_size = 1;
        do
        {
            free(variables);
            variables = malloc(variables_size);
            if(!variables)
                return -1;

            ret = NtEnumerateSystemEnvironmentValuesEx(SystemEnvironmentNameInformation, variables, &variables_size);
            last_winapi_function = _T("NtEnumerateSystemEnvironmentValuesEx");
        } while(ret == STATUS_BUFFER_TOO_SMALL || ret == STATUS_INFO_LENGTH_MISMATCH);

        if(ret < 0)
            return ret;
    }

    if(efi_get_next_variable_name_progress_cb)
        efi_get_next_variable_name_progress_cb(current_offset, variables_size);

    if(current_offset >= variables_size)
    {
        current_offset = 0u;
        return 0;
    }

    const variable_info_t *variable = advance_bytes(variables, current_offset);
    if(!variable->NextEntryOffset)
        current_offset = variables_size;
    else
        current_offset += variable->NextEntryOffset;

    if(_sntprintf_s(current_guid.data, 39, 39, _T("{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}"),
           variable->VendorGUID.Data1, variable->VendorGUID.Data2, variable->VendorGUID.Data3,
           variable->VendorGUID.Data4[0], variable->VendorGUID.Data4[1], variable->VendorGUID.Data4[2], variable->VendorGUID.Data4[3],
           variable->VendorGUID.Data4[4], variable->VendorGUID.Data4[5], variable->VendorGUID.Data4[6], variable->VendorGUID.Data4[7])
        != 38)
        return -1;

    *guid = &current_guid;
    *name = (TCHAR *)variable->Name;
    return 1;
}

int efi_guid_cmp(const efi_guid_t *a, const efi_guid_t *b)
{
    return _tcsncmp(a->data, b->data, sizeof(a->data) / sizeof(a->data[0]));
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
    *function = last_winapi_function;
    if(!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_buffer, 1024, nullptr))
        return -1;

    *message = error_buffer;
    return 1;
}

void efi_error_clear(void)
{
    // Nothing to do
}
