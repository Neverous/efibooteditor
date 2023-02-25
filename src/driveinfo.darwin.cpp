// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DiskArbitration/DiskArbitration.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOPartitionScheme.h>
#include <QDir>

#include "compat.h"
#include "driveinfo.h"

auto DriveInfo::getAll(bool refresh) -> QVector<DriveInfo>
{
    if(!refresh && !all.empty())
        return all;

    all.clear();

    QDir disks("/dev");
    if(!disks.exists())
        return all;

    disks.setFilter(QDir::Files | QDir::System);
    disks.setNameFilters({"disk*s*"});
    auto session_cf = DASessionCreate(kCFAllocatorDefault);
    if(session_cf == nullptr)
        return all;

    for(const auto &disk: disks.entryInfoList())
    {
        DriveInfo driveinfo{};
        driveinfo.name = disk.fileName();
        auto disk_cf = DADiskCreateFromBSDName(kCFAllocatorDefault, session_cf, disk.filePath().toStdString().c_str());
        if(disk_cf == nullptr)
            continue;

        auto disk_info_cf = DADiskCopyDescription(disk_cf);
        if(disk_info_cf == nullptr)
        {
            CFRelease(disk_cf);
            continue;
        }

        auto disk_service_io = DADiskCopyIOMedia(disk_cf);
        if(disk_service_io == 0)
        {
            CFRelease(disk_info_cf);
            CFRelease(disk_cf);
            continue;
        }

        CFRelease(disk_cf);
        CFTypeRef value_cf = CFDictionaryGetValue(disk_info_cf, kDADiskDescriptionVolumeNameKey);
        if(value_cf != nullptr)
            driveinfo.name = QString::fromCFString((CFStringRef)value_cf);

        value_cf = CFDictionaryGetValue(disk_info_cf, kDADiskDescriptionVolumeUUIDKey);
        if(value_cf == nullptr)
            value_cf = CFDictionaryGetValue(disk_info_cf, kDADiskDescriptionMediaUUIDKey);

        if(value_cf != nullptr)
        {
            // Assume GPT
            driveinfo.signature_type = DriveInfo::SIGNATURE::GUID;
            driveinfo.signature = QUuid::fromCFUUID((CFUUIDRef)value_cf);
        }

        value_cf = IORegistryEntryCreateCFProperty(disk_service_io, CFSTR(kIOMediaPartitionIDKey), kCFAllocatorDefault, 0);
        if(value_cf != nullptr)
        {
            CFNumberGetValue((CFNumberRef)value_cf, CFNumberGetType((CFNumberRef)value_cf), static_cast<void *>(&driveinfo.partition));
            CFRelease(value_cf);
        }

        uint32_t block_size = 0;
        value_cf = IORegistryEntryCreateCFProperty(disk_service_io, CFSTR(kIOMediaPreferredBlockSizeKey), kCFAllocatorDefault, 0);
        if(value_cf != nullptr)
        {
            CFNumberGetValue((CFNumberRef)value_cf, CFNumberGetType((CFNumberRef)value_cf), static_cast<void *>(&block_size));
            CFRelease(value_cf);
        }

        value_cf = IORegistryEntryCreateCFProperty(disk_service_io, CFSTR(kIOMediaBaseKey), kCFAllocatorDefault, 0);
        if(value_cf != nullptr)
        {
            CFNumberGetValue((CFNumberRef)value_cf, CFNumberGetType((CFNumberRef)value_cf), static_cast<void *>(&driveinfo.start));
            CFRelease(value_cf);
        }

        if(block_size > 0u)
            driveinfo.start /= block_size;

        value_cf = CFDictionaryGetValue(disk_info_cf, kDADiskDescriptionMediaSizeKey);
        if(value_cf != nullptr)
            CFNumberGetValue((CFNumberRef)value_cf, kCFNumberIntType, &driveinfo.size);

        IOObjectRelease(disk_service_io);
        CFRelease(disk_info_cf);
        all.append(driveinfo);
    }

    CFRelease(session_cf);
    std::sort(std::begin(all), std::end(all));
    return all;
}
