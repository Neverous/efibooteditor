// SPDX-License-Identifier: LGPL-3.0-or-later
#include <Windows.h>

#include "compat.h"
#include "driveinfo.h"

QVector<DriveInfo> DriveInfo::getAll(bool refresh)
{
    if(!refresh && !all.empty())
        return all;

    all.clear();

    TCHAR volume_name[MAX_PATH];
    HANDLE volume_handle = FindFirstVolume(volume_name, ARRAYSIZE(volume_name));

    if(volume_handle == INVALID_HANDLE_VALUE)
        return all;

    for(BOOL volume_found = true; volume_found; volume_found = FindNextVolume(volume_handle, volume_name, ARRAYSIZE(volume_name)))
    {
        DriveInfo driveinfo{};
        TCHAR device_name[MAX_PATH];
        size_t length = _tcslen(volume_name);
        if(length != 49u)
            continue;

        volume_name[length - 1u] = _T('\0');
        if(!QueryDosDevice(&volume_name[4], device_name, ARRAYSIZE(device_name)))
            continue;

        volume_name[length - 1u] = _T('\\');

        PARTITION_INFORMATION_EX partition_information;

        device_name[4] = _T('\\');
        device_name[5] = _T('\\');
        device_name[6] = _T('.');
        HANDLE device_handle = CreateFile(device_name + 4, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        device_name[4] = _T('i');
        device_name[5] = _T('c');
        device_name[6] = _T('e');
        if(device_handle == INVALID_HANDLE_VALUE)
            continue;

        if(!DeviceIoControl(device_handle, IOCTL_DISK_GET_PARTITION_INFO_EX, nullptr, 0, &partition_information, sizeof(partition_information), nullptr, 0))
        {
            CloseHandle(device_handle);
            continue;
        }

        CloseHandle(device_handle);
        driveinfo.name = QStringFromTCharArray(device_name + 8);
        switch(partition_information.PartitionStyle)
        {
        case PARTITION_STYLE_GPT:
            driveinfo.signature_type = DriveInfo::SIGNATURE::GUID;
            driveinfo.name += QString(" (%1)").arg(QString::fromWCharArray(partition_information.Gpt.Name));
            driveinfo.signature = partition_information.Gpt.PartitionId;
            break;

        case PARTITION_STYLE_MBR:
            driveinfo.signature_type = DriveInfo::SIGNATURE::MBR;
            driveinfo.signature = partition_information.Mbr.PartitionId;
            break;

        case PARTITION_STYLE_RAW:
            driveinfo.signature_type = DriveInfo::SIGNATURE::NONE;
            break;
        }

        driveinfo.partition = partition_information.PartitionNumber;
        driveinfo.start = static_cast<uint64_t>(partition_information.StartingOffset.QuadPart);
        driveinfo.size = static_cast<uint64_t>(partition_information.PartitionLength.QuadPart);
        all.append(driveinfo);
    }

    FindVolumeClose(volume_handle);
    std::sort(std::begin(all), std::end(all));
    return all;
}
