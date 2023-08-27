// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compat.h"
#include "efivar-lite.common.h"
#include "efivar-lite/efiboot-loadopt.h"

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

const size_t EFI_MAX_VARIABLES = sizeof(variable_names) / sizeof(variable_names[0]) + sizeof(enumerated_variable_names) / sizeof(enumerated_variable_names[0]) * 65536u;

static size_t current_variable = 0u;
int _efi_get_next_variable_name(efi_guid_t **guid, TCHAR **name)
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
    if(index / 65536u < sizeof(enumerated_variable_names) / sizeof(enumerated_variable_names[0]))
    {
        size_t enum_index = index / 65536u;
        size_t enum_value = index - enum_index * 65536u;
        *guid = (efi_guid_t *)&efi_guid_global;
        _sntprintf_s(variable_name_buffer, 32, 31, _T("%s%04zX"), enumerated_variable_names[enum_index], enum_value);
        *name = (TCHAR *)&variable_name_buffer;
        ++current_variable;
        return 1;
    }

    index -= sizeof(enumerated_variable_names) / sizeof(enumerated_variable_names[0]) * 65536u;
    if(index == 0u)
    {
        current_variable = 0u;
        return 0;
    }

    return -1;
}

efidp efi_loadopt_path(efi_load_option *opt, ssize_t limit)
{
    uint8_t *ptr = (uint8_t *)opt;
    if((size_t)limit <= offsetof(efi_load_option, description))
        return NULL;

    limit -= (ssize_t)offsetof(efi_load_option, description);
    ptr += offsetof(efi_load_option, description);
    for(size_t d = 0; limit > 0 && opt->description[d]; ++d, limit -= (ssize_t)sizeof(opt->description[0]), ptr += sizeof(opt->description[0]))
        ;
    // \0
    limit -= (ssize_t)sizeof(opt->description[0]);
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
        if(len > (size_t)limit)
            return 0;

        if((size_t)limit - len < offsetof(efi_load_option, file_path_list_length))
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
