// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "../compat.h"
#include "efivar.h"
#include <errno.h>
#include <limits.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t type;
    uint8_t subtype;
    uint16_t length;
} efidp_header;

typedef uint16_t efidp_type_subtype;

typedef uint8_t efidp_boolean;

enum EFIDP_TYPE
{
    EFIDP_TYPE_HW = 0x01,
    EFIDP_TYPE_ACPI = 0x02,
    EFIDP_TYPE_MSG = 0x03,
    EFIDP_TYPE_MEDIA = 0x04,
    EFIDP_TYPE_END = 0x7f,
};

enum EFIDP_HW
{
    EFIDP_HW_PCI = 0x01,
};

enum EFIDP_ACPI
{
    EFIDP_ACPI_HID = 0x01,
};

enum EFIDP_MSG
{
    EFIDP_MSG_SATA = 0x12,
};

enum EFIDP_MEDIA
{
    EFIDP_MEDIA_HD = 0x1,
    EFIDP_MEDIA_FILE = 0x4,
    EFIDP_MEDIA_FIRMWARE_FILE = 0x6,
    EFIDP_MEDIA_FIRMWARE_VOLUME = 0x7,
};

typedef struct
{
    efidp_header header;
    uint8_t function;
    uint8_t device;
} efidp_pci;

typedef struct
{
    efidp_header header;
    uint32_t hid;
    uint32_t uid;
} efidp_hid;

typedef struct
{
    efidp_header header;
    uint16_t hba_port;
    uint16_t port_multiplier_port;
    uint16_t lun;
} efidp_sata;

typedef struct
{
    efidp_header header;
    uint32_t partition_number;
    uint64_t partition_start;
    uint64_t partition_size;
    uint8_t partition_signature[16];
    uint8_t partition_format;
    uint8_t signature_type;
#ifdef __ia64
    uint8_t _padding[6]; /* Emperically needed */
#endif
} efidp_hd;

enum EFIDP_HD_FORMAT
{
    EFIDP_HD_FORMAT_PCAT = 0x01,
    EFIDP_HD_FORMAT_GPT = 0x02,
};

enum EFIDP_HD_SIGNATURE
{
    EFIDP_HD_SIGNATURE_NONE = 0x00,
    EFIDP_HD_SIGNATURE_MBR = 0x01,
    EFIDP_HD_SIGNATURE_GUID = 0x02,
};

typedef struct
{
    efidp_header header;
    uint16_t name[1];
} efidp_file;

typedef struct
{
    efidp_header header;
    uint8_t name[16];
} efidp_firmware_file;

typedef struct
{
    efidp_header header;
    uint8_t name[16];
} efidp_firmware_volume;

enum EFIDP_END
{
    EFIDP_END_ENTIRE = 0xff,
    EFIDP_END_INSTANCE = 0x01,
};

/* utility functions */
typedef union {
    efidp_header header;
    efidp_type_subtype _type_subtype;
    efidp_pci pci;
    efidp_hid hid;
    efidp_sata sata;
    efidp_hd hd;
    efidp_file file;
    efidp_firmware_file firmware_file;
    efidp_firmware_volume firmware_volume;
} efidp_data;
typedef efidp_data *efidp;
typedef const efidp_data *const_efidp;

#pragma pack(pop)

static inline int16_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED
    efidp_type(const_efidp dp)
{
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    return dp->header.type;
}

static inline int16_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED
    efidp_subtype(const_efidp dp)
{
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    return dp->header.subtype;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_node_size(const_efidp dn)
{
    if(ATTR_NONNULL_IS_NULL(dn) || dn->header.length < 4)
    {
        errno = EINVAL;
        return -1;
    }

    return dn->header.length;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_next_node(const_efidp in, const_efidp *out)
{
    if(efidp_type(in) == EFIDP_TYPE_END && efidp_subtype(in) == EFIDP_END_ENTIRE)
        return 0;

    ssize_t sz = efidp_node_size(in);
    if(sz < 0)
        return -1;

    *out = (const_efidp)((const uint8_t *)in + sz);
    if(*out < in)
    {
        errno = EINVAL;
        return -1;
    }

    return 1;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_next_instance(const_efidp in, const_efidp *out)
{
    if(efidp_type(in) != EFIDP_TYPE_END || efidp_subtype(in) != EFIDP_END_INSTANCE)
    {
        errno = EINVAL;
        return -1;
    }

    ssize_t sz = efidp_node_size(in);
    if(sz < 0)
        return -1;

    *out = (const_efidp)((const uint8_t *)in + sz);
    if(*out < in)
    {
        errno = EINVAL;
        return -1;
    }

    return 1;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_is_multiinstance(const_efidp dn)
{
    while(1)
    {
        const_efidp next = NULL;
        int rc = efidp_next_node(dn, &next);
        if(rc < 0)
        {
            errno = EINVAL;
            return -1;
        }
        else if(rc == 0)
            return 0;

        dn = next;
        if(efidp_type(dn) == EFIDP_TYPE_END && efidp_subtype(dn) == EFIDP_END_INSTANCE)
            return 1;

        if(efidp_type(dn) == EFIDP_TYPE_END && efidp_subtype(dn) == EFIDP_END_ENTIRE)
            return 0;
    }

    return 0;
}

static inline int
    ATTR_ARTIFICIAL ATTR_NONNULL(1, 2) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_get_next_end(const_efidp in, const_efidp *out)
{
    while(1)
    {
        if(efidp_type(in) == EFIDP_TYPE_END)
        {
            *out = in;
            return 0;
        }

        ssize_t sz = efidp_node_size(in);
        if(sz < 0)
            break;

        const_efidp next = (const_efidp)((const uint8_t *)in + sz);
        if(next < in)
        {
            errno = EINVAL;
            return -1;
        }

        in = next;
    }

    return -1;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_size(const_efidp dp)
{
    ssize_t size = 0;
    if(ATTR_NONNULL_IS_NULL(dp))
    {
        errno = EINVAL;
        return -1;
    }

    if(efidp_type(dp) == EFIDP_TYPE_END && efidp_subtype(dp) == EFIDP_END_ENTIRE)
        return efidp_node_size(dp);

    while(1)
    {
        ssize_t sz = efidp_node_size(dp);
        if(sz < 0)
            return sz;

        size += sz;
        const_efidp next = NULL;
        int rc = efidp_next_instance(dp, &next);
        if(rc < 0)
        {
            rc = efidp_next_node(dp, &next);
            if(rc < 0)
                return rc;

            if(rc == 0)
                break;
        }

        dp = next;
    }

    return size;
}

static inline ssize_t
    ATTR_ARTIFICIAL ATTR_NONNULL(1) ATTR_UNUSED ATTR_WARN_UNUSED_RESULT
    efidp_instance_size(const_efidp dpi)
{
    ssize_t size = 0;
    while(1)
    {
        ssize_t sz = efidp_node_size(dpi);
        if(sz < 0)
            return sz;

        size += sz;
        if(efidp_type(dpi) == EFIDP_TYPE_END)
            break;

        const_efidp next = NULL;
        int rc = efidp_next_node(dpi, &next);
        if(rc < 0)
            return rc;

        dpi = next;
    }

    return size;
}
